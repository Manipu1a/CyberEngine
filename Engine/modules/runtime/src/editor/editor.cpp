#include "editor/editor.h"
#include "imgui/backends/imgui_impl_dx12.h"
#include "platform/memory.h"
#include "platform/configure.h"
#include "graphics/backend/d3d12/render_device_d3d12.h"
#include "graphics/backend/d3d12/command_buffer_d3d12.h"
#include "core/debug.h"
#include "log/Log.h"
#include "editor/imgui_renderer.h"
#include "editor/imgui_log_sink.h"
#include "editor/resource_type_registry.h"
#include "asset/asset.h"
#include "renderer/renderer.h"
#include "gameruntime/sampleapp.h"
#include "gameruntime/world.h"
#include "gameruntime/mesh.h"
#include "gameruntime/scene_node.h"
#include "gameruntime/scene_serializer.h"
#include "component/primitive.h"
#include "component/mesh_component.h"
#include "component/camera_component.h"
#include "component/directional_light_component.h"
#include "core/file_helper.hpp"
#include <array>
#include <filesystem>
#include <limits>
#include <string>
#include <string_view>
#include <vector>
#include <cstring>
#include <cctype>
#include <algorithm>
#include <fstream>

#ifdef CYBER_RUNTIME_PLATFORM_WINDOWS
#include <commdlg.h>
#include <objbase.h>
#include <shellapi.h>
#include <wincodec.h>
#include <wrl/client.h>
#endif

namespace Cyber
{
    namespace Editor
    {
        namespace
        {
#ifdef CYBER_RUNTIME_PLATFORM_WINDOWS
            class ScopedComApartment
            {
            public:
                ScopedComApartment()
                    : m_hr(CoInitializeEx(nullptr, COINIT_MULTITHREADED))
                    , m_should_uninitialize(SUCCEEDED(m_hr))
                {
                }

                ~ScopedComApartment()
                {
                    if (m_should_uninitialize)
                        CoUninitialize();
                }

                ScopedComApartment(const ScopedComApartment&) = delete;
                ScopedComApartment& operator=(const ScopedComApartment&) = delete;

                bool is_available() const
                {
                    return SUCCEEDED(m_hr) || m_hr == RPC_E_CHANGED_MODE;
                }

            private:
                HRESULT m_hr = E_FAIL;
                bool m_should_uninitialize = false;
            };

            bool load_png_rgba_with_wic(const std::filesystem::path& path,
                                        std::vector<uint8_t>& pixels,
                                        uint32_t& width,
                                        uint32_t& height)
            {
                pixels.clear();
                width = 0;
                height = 0;

                ScopedComApartment com;
                if (!com.is_available())
                {
                    CB_WARN("Failed to initialize COM while loading editor icon: {}", path.string().c_str());
                    return false;
                }

                Microsoft::WRL::ComPtr<IWICImagingFactory> factory;
                HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
                                              IID_PPV_ARGS(factory.GetAddressOf()));
                if (FAILED(hr))
                {
                    CB_WARN("Failed to create WIC factory while loading editor icon: {}", path.string().c_str());
                    return false;
                }

                Microsoft::WRL::ComPtr<IWICBitmapDecoder> decoder;
                hr = factory->CreateDecoderFromFilename(path.c_str(), nullptr, GENERIC_READ,
                                                        WICDecodeMetadataCacheOnLoad,
                                                        decoder.GetAddressOf());
                if (FAILED(hr))
                {
                    CB_WARN("Failed to open editor icon: {}", path.string().c_str());
                    return false;
                }

                Microsoft::WRL::ComPtr<IWICBitmapFrameDecode> frame;
                hr = decoder->GetFrame(0, frame.GetAddressOf());
                if (FAILED(hr))
                {
                    CB_WARN("Failed to decode editor icon frame: {}", path.string().c_str());
                    return false;
                }

                Microsoft::WRL::ComPtr<IWICFormatConverter> converter;
                hr = factory->CreateFormatConverter(converter.GetAddressOf());
                if (FAILED(hr))
                {
                    CB_WARN("Failed to create WIC format converter: {}", path.string().c_str());
                    return false;
                }

                hr = converter->Initialize(frame.Get(), GUID_WICPixelFormat32bppRGBA,
                                           WICBitmapDitherTypeNone, nullptr, 0.0,
                                           WICBitmapPaletteTypeCustom);
                if (FAILED(hr))
                {
                    CB_WARN("Failed to convert editor icon to RGBA: {}", path.string().c_str());
                    return false;
                }

                UINT w = 0;
                UINT h = 0;
                hr = converter->GetSize(&w, &h);
                if (FAILED(hr) || w == 0 || h == 0)
                {
                    CB_WARN("Invalid editor icon size: {}", path.string().c_str());
                    return false;
                }

                const uint64_t stride = static_cast<uint64_t>(w) * 4ull;
                const uint64_t total_size = stride * static_cast<uint64_t>(h);
                if (total_size > static_cast<uint64_t>(std::numeric_limits<UINT>::max()))
                {
                    CB_WARN("Editor icon is too large: {}", path.string().c_str());
                    return false;
                }

                pixels.resize(static_cast<size_t>(total_size));
                hr = converter->CopyPixels(nullptr, static_cast<UINT>(stride),
                                           static_cast<UINT>(pixels.size()), pixels.data());
                if (FAILED(hr))
                {
                    CB_WARN("Failed to copy editor icon pixels: {}", path.string().c_str());
                    pixels.clear();
                    return false;
                }

                width = w;
                height = h;
                return true;
            }

            bool query_image_size_with_wic(const std::filesystem::path& path,
                                           uint32_t& width,
                                           uint32_t& height)
            {
                width = 0;
                height = 0;

                ScopedComApartment com;
                if (!com.is_available())
                    return false;

                Microsoft::WRL::ComPtr<IWICImagingFactory> factory;
                HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
                                              IID_PPV_ARGS(factory.GetAddressOf()));
                if (FAILED(hr))
                    return false;

                Microsoft::WRL::ComPtr<IWICBitmapDecoder> decoder;
                hr = factory->CreateDecoderFromFilename(path.c_str(), nullptr, GENERIC_READ,
                                                        WICDecodeMetadataCacheOnDemand,
                                                        decoder.GetAddressOf());
                if (FAILED(hr))
                    return false;

                Microsoft::WRL::ComPtr<IWICBitmapFrameDecode> frame;
                hr = decoder->GetFrame(0, frame.GetAddressOf());
                if (FAILED(hr))
                    return false;

                UINT w = 0;
                UINT h = 0;
                hr = frame->GetSize(&w, &h);
                if (FAILED(hr) || w == 0 || h == 0)
                    return false;

                width = w;
                height = h;
                return true;
            }

            bool open_image_import_dialog(HWND owner, std::filesystem::path& out_path)
            {
                std::array<wchar_t, 4096> file_name = {};
                static constexpr wchar_t kImageFilter[] =
                    L"Image Files (*.png;*.jpg;*.jpeg;*.tif;*.tiff;*.sgi;*.dds;*.ktx;*.hdr)\0"
                    L"*.png;*.jpg;*.jpeg;*.tif;*.tiff;*.sgi;*.dds;*.ktx;*.hdr\0"
                    L"All Files (*.*)\0*.*\0";

                OPENFILENAMEW ofn = {};
                ofn.lStructSize = sizeof(ofn);
                ofn.hwndOwner = owner;
                ofn.lpstrTitle = L"Import Image";
                ofn.lpstrFilter = kImageFilter;
                ofn.lpstrFile = file_name.data();
                ofn.nMaxFile = static_cast<DWORD>(file_name.size());
                ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;

                if (!GetOpenFileNameW(&ofn))
                {
                    DWORD error = CommDlgExtendedError();
                    if (error != 0)
                        CB_WARN("Image import dialog failed: 0x{:08x}", error);
                    return false;
                }

                out_path = file_name.data();
                return true;
            }

            bool open_mesh_import_dialog(HWND owner, std::filesystem::path& out_path)
            {
                std::array<wchar_t, 4096> file_name = {};
                static constexpr wchar_t kMeshFilter[] =
                    L"Mesh Files (*.fbx;*.gltf;*.glb)\0"
                    L"*.fbx;*.gltf;*.glb\0"
                    L"All Files (*.*)\0*.*\0";

                OPENFILENAMEW ofn = {};
                ofn.lStructSize = sizeof(ofn);
                ofn.hwndOwner = owner;
                ofn.lpstrTitle = L"Import Mesh";
                ofn.lpstrFilter = kMeshFilter;
                ofn.lpstrFile = file_name.data();
                ofn.nMaxFile = static_cast<DWORD>(file_name.size());
                ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;

                if (!GetOpenFileNameW(&ofn))
                {
                    DWORD error = CommDlgExtendedError();
                    if (error != 0)
                        CB_WARN("Mesh import dialog failed: 0x{:08x}", error);
                    return false;
                }

                out_path = file_name.data();
                return true;
            }
#endif
        }

        Editor::Editor(const EditorCreateInfo& createInfo)
        {
            m_imguiRenderer = cyber_new<ImGuiRenderer>(createInfo);

            // Create and register ImGui log sink
            m_imgui_log_sink = std::make_shared<ImGuiLogSink>(&log);
            m_imgui_log_sink->set_level(spdlog::level::trace);
            Log::addSink(m_imgui_log_sink);

            //initialize(createInfo.pDevice, createInfo.Hwnd);
        }

        Editor::~Editor()
        {
            // The sink will be destroyed automatically with shared_ptr
        }

        void Editor::initialize(RenderObject::IRenderDevice* device, HWND hwnd)
        {
            m_hwnd = hwnd;

            // Setup Dear ImGui context
            IMGUI_CHECKVERSION();
            imgui_context = ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO();
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
            io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
            io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

            // Setup Dear ImGui style
            ImGui::StyleColorsDark();

            // Populate the content-browser resource-type whitelist and root
            // the directory tree at the active sample/project content folder.
            ResourceTypeRegistry::seed_defaults();
            refresh_content_browser_root(true);

            // Setup Platform/Renderer backends
            m_imguiRenderer->initialize();
            load_content_browser_icons(device);
        }
        /*
        void Editor::run()
        {

        }
        */
        void Editor::new_frame(uint32_t renderSurfaceWidth, uint32_t renderSurfaceHeight)
        {
            // Start the Dear ImGui frame
            //ImGui_ImplDX12_NewFrame();

            m_imguiRenderer->new_frame(renderSurfaceWidth, renderSurfaceHeight);
            ImGui::NewFrame();
            
            // Initialize ImGuizmo for this frame
            ImGuizmo::BeginFrame();
        }

        void Editor::update(float deltaTime)
        {
            //ImGui_ImplWin32_NewFrame();
            //ImGui::NewFrame();
            bool show_demo_window = true;
            //ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
            
            static bool opt_fullscreen = true;
            static bool opt_padding = false;
            static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

            // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
            // because it would be confusing to have two docking targets within each others.
            ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
            if (opt_fullscreen)
            {
                const ImGuiViewport* viewport = ImGui::GetMainViewport();
                ImGui::SetNextWindowPos(viewport->WorkPos);
                ImGui::SetNextWindowSize(viewport->WorkSize);
                ImGui::SetNextWindowViewport(viewport->ID);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
                window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
                window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
            }
            else
            {
                dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
            }

            // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
            // and handle the pass-thru hole, so we ask Begin() to not render a background.
            if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
                window_flags |= ImGuiWindowFlags_NoBackground;

            // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
            // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
            // all active windows docked into it will lose their parent and become undocked.
            // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
            // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
            if (!opt_padding)
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
            ImGui::Begin("DockSpace Demo", nullptr, window_flags);
            if (!opt_padding)
                ImGui::PopStyleVar();

            if (opt_fullscreen)
                ImGui::PopStyleVar(2);

            // Submit the DockSpace
            ImGuiIO& io = ImGui::GetIO();
            ImGuiID dockspace_id = 0;
            if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
            {
                dockspace_id = ImGui::GetID("MyDockSpace");

                // Build default layout on first run, before DockSpace() creates an empty node:
                //   +-------------------------------+---------+
                //   |                               |         |
                //   |           Viewport            | Details |
                //   |                               |         |
                //   +-------------------------------+---------+
                //   |   Content Browser  |   Log              |
                //   +--------------------+--------------------+
                if (!m_dock_layout_built && ImGui::DockBuilderGetNode(dockspace_id) == nullptr)
                {
                    ImGui::DockBuilderRemoveNode(dockspace_id);
                    ImGui::DockBuilderAddNode(dockspace_id, dockspace_flags | ImGuiDockNodeFlags_DockSpace);
                    ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->WorkSize);

                    ImGuiID dock_main = dockspace_id;
                    ImGuiID dock_right = ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Right, 0.22f, nullptr, &dock_main);
                    ImGuiID dock_bottom = ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Down, 0.28f, nullptr, &dock_main);
                    ImGuiID dock_bottom_right = ImGui::DockBuilderSplitNode(dock_bottom, ImGuiDir_Right, 0.5f, nullptr, &dock_bottom);
                    ImGuiID dock_right_bottom = ImGui::DockBuilderSplitNode(dock_right, ImGuiDir_Down, 0.5f, nullptr, &dock_right);

                    ImGui::DockBuilderDockWindow("Viewport", dock_main);
                    ImGui::DockBuilderDockWindow("Scene Hierarchy", dock_right);
                    ImGui::DockBuilderDockWindow("Details", dock_right_bottom);
                    ImGui::DockBuilderDockWindow("Content Browser", dock_bottom);
                    ImGui::DockBuilderDockWindow("Log", dock_bottom_right);

                    ImGui::DockBuilderFinish(dockspace_id);
                    m_dock_layout_built = true;
                }

                ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
            }
            
            if (ImGui::BeginMenuBar())
            {
                if (ImGui::BeginMenu("File"))
                {
                    // Open is enabled whenever the currently-selected Content
                    // Browser file looks like a .scene. We reuse that
                    // selection rather than pop a native file dialog.
                    bool can_open = false;
                    if (!m_selected_asset.empty())
                    {
                        std::string ext = std::filesystem::path(m_selected_asset).extension().string();
                        std::transform(ext.begin(), ext.end(), ext.begin(),
                            [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
                        can_open = (ext == ".scene");
                    }
                    if (ImGui::MenuItem("Open Scene", nullptr, false, can_open))
                        handle_open_scene();

                    Core::Application* app_fm = m_pApp ? m_pApp : Core::Application::getApp();
                    Samples::SampleApp* sample_fm = app_fm ? app_fm->get_sample_app() : nullptr;
                    RefCntAutoPtr<World> world_fm = sample_fm ? sample_fm->get_world() : RefCntAutoPtr<World>{};
                    const bool has_source = world_fm && !world_fm->source_path().empty();

                    if (ImGui::MenuItem("Save Scene", nullptr, false, (bool)world_fm && has_source))
                        handle_save_scene();

                    if (ImGui::MenuItem("Save Scene As...", nullptr, false, (bool)world_fm))
                    {
                        if (world_fm && !world_fm->source_path().empty())
                            std::snprintf(m_save_as_buffer, sizeof(m_save_as_buffer), "%s", world_fm->source_path().c_str());
                        else
                            m_save_as_buffer[0] = '\0';
                        m_show_save_as_popup = true;
                    }

                    ImGui::Separator();
                    if (ImGui::MenuItem("Import Image..."))
                        handle_import_image();
                    if (ImGui::MenuItem("Import Mesh..."))
                        handle_import_mesh();

                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Options"))
                {
                    // Disabling fullscreen would allow the window to be moved to the front of other windows,
                    // which we can't undo at the moment without finer window depth/z control.
                    ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen);
                    ImGui::MenuItem("Padding", NULL, &opt_padding);
                    ImGui::Separator();

                    if (ImGui::MenuItem("Flag: NoDockingOverCentralNode", "", (dockspace_flags & ImGuiDockNodeFlags_NoDockingOverCentralNode) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_NoDockingOverCentralNode; }
                    if (ImGui::MenuItem("Flag: NoDockingSplit",         "", (dockspace_flags & ImGuiDockNodeFlags_NoDockingSplit) != 0))             { dockspace_flags ^= ImGuiDockNodeFlags_NoDockingSplit; }
                    if (ImGui::MenuItem("Flag: NoUndocking",            "", (dockspace_flags & ImGuiDockNodeFlags_NoUndocking) != 0))                { dockspace_flags ^= ImGuiDockNodeFlags_NoUndocking; }
                    if (ImGui::MenuItem("Flag: NoResize",               "", (dockspace_flags & ImGuiDockNodeFlags_NoResize) != 0))                   { dockspace_flags ^= ImGuiDockNodeFlags_NoResize; }
                    if (ImGui::MenuItem("Flag: AutoHideTabBar",         "", (dockspace_flags & ImGuiDockNodeFlags_AutoHideTabBar) != 0))             { dockspace_flags ^= ImGuiDockNodeFlags_AutoHideTabBar; }
                    if (ImGui::MenuItem("Flag: PassthruCentralNode",    "", (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode) != 0, opt_fullscreen)) { dockspace_flags ^= ImGuiDockNodeFlags_PassthruCentralNode; }
                    ImGui::Separator();

                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("View"))
                {
                    ImGui::MenuItem("Scene Hierarchy", NULL, &m_show_scene_hierarchy);
                    ImGui::MenuItem("Content Browser", NULL, &m_show_content_browser);
                    ImGui::MenuItem("Details",         NULL, &m_show_details_panel);
                    ImGui::EndMenu();
                }

                ImGui::EndMenuBar();
            }
            ImGui::End();
            
            // show scene
            if(ImGui::Begin("Viewport"))
            {
                if(ImGui::BeginChild("ViewportChild"))
                {
                    auto* renderer = Core::Application::getApp()->get_renderer();
                    auto* render_device = renderer->get_render_device();
                    auto back_buffer_index = renderer->get_back_buffer_index();
                    auto& scene_target = renderer->get_scene_target(back_buffer_index);
                    auto* color_buffer = scene_target.color_buffer->get_default_texture_view(TEXTURE_VIEW_SHADER_RESOURCE);
                    auto& color_buffer_desc = scene_target.color_buffer->get_create_desc();
                    ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // No tint
                    ImVec4 border_col = ImGui::GetStyleColorVec4(ImGuiCol_Border);
                    ImVec2 viewport_size = ImGui::GetContentRegionAvail();
                    renderer->resize_viewport(viewport_size.x, viewport_size.y);
                    //renderer->resize_swap_chain(viewport_size.x, viewport_size.y);
                    //render_device->bind_texture_view(color_buffer);
                    ImGui::Image((ImTextureID)color_buffer, ImVec2(viewport_size.x, viewport_size.y), ImVec2(0, 0), ImVec2(1, 1), tint_col, border_col);

                    // Viewport is also a drag-drop target for asset files.
                    if (ImGui::BeginDragDropTarget())
                    {
                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CYBER_ASSET"))
                        {
                            const char* path = static_cast<const char*>(payload->Data);
                            try_accept_asset_drop(path, float3{0.0f, 0.0f, 0.0f});
                        }
                        ImGui::EndDragDropTarget();
                    }

                    // Object manipulation gizmo for the Scene Hierarchy selection.
                    // Gizmo operates on the currently selected component — if the
                    // user has only highlighted a node (no component), no gizmo is
                    // shown (a node without a Primitive doesn't have a transform).
                    if (m_projection_matrix_set && m_selected_node_id != 0 && m_selected_component_index >= 0)
                    {
                        Core::Application* app_for_gizmo = m_pApp ? m_pApp : Core::Application::getApp();
                        Samples::SampleApp* sample_app = app_for_gizmo ? app_for_gizmo->get_sample_app() : nullptr;
                        RefCntAutoPtr<World> world = sample_app ? sample_app->get_world() : RefCntAutoPtr<World>{};
                        if (world)
                        {
                            SceneNode* node = world->find_node(m_selected_node_id);
                            Component::Primitive* prim = nullptr;
                            if (node && (size_t)m_selected_component_index < node->components.size())
                                prim = node->components[m_selected_component_index].get();

                            if (prim)
                            {
                                ImVec2 image_min = ImGui::GetItemRectMin();
                                ImVec2 image_size = ImGui::GetItemRectSize();

                                ImGuizmo::SetOrthographic(false);
                                ImGuizmo::SetDrawlist();
                                ImGuizmo::SetRect(image_min.x, image_min.y, image_size.x, image_size.y);

                                float4x4 matrix = prim->local_matrix();

                                ImGuizmo::Manipulate(
                                    m_view_matrix,
                                    m_projection_matrix,
                                    m_gizmo_op,
                                    (m_gizmo_op == ImGuizmo::SCALE) ? ImGuizmo::LOCAL : m_gizmo_mode,
                                    matrix.Data());

                                if (ImGuizmo::IsUsing())
                                {
                                    float translation[3] = {};
                                    float rotation_euler[3] = {};
                                    float scale[3] = {};
                                    ImGuizmo::DecomposeMatrixToComponents(matrix.Data(),
                                                                          translation, rotation_euler, scale);
                                    prim->position = float3(translation[0], translation[1], translation[2]);
                                    prim->scale    = float3(scale[0],       scale[1],       scale[2]);

                                    const float deg2rad = 3.14159265358979323846f / 180.0f;
                                    quaternion_f qx = quaternion_f::rotation_from_axis_angle(float3(1,0,0), rotation_euler[0] * deg2rad);
                                    quaternion_f qy = quaternion_f::rotation_from_axis_angle(float3(0,1,0), rotation_euler[1] * deg2rad);
                                    quaternion_f qz = quaternion_f::rotation_from_axis_angle(float3(0,0,1), rotation_euler[2] * deg2rad);
                                    prim->rotation = quaternion_f::mul(quaternion_f::mul(qz, qy), qx);
                                    world->set_dirty(true);
                                }
                            }
                        }
                    }

                    // Draw world coordinate arrow gizmo in bottom-left corner
                    ImVec2 window_pos = ImGui::GetWindowPos();
                    ImVec2 window_size = ImGui::GetWindowSize();

                    ImVec2 bottom_left = ImVec2(window_pos.x, window_pos.y + window_size.y);

                    // Set gizmo position and size (bottom-left corner)
                    float gizmo_size = 100.0f;
                    float gizmo_offset = 50.0f;
                    ImVec2 gizmo_center = ImVec2(bottom_left.x + gizmo_offset, 
                                                 bottom_left.y - gizmo_offset);
                    
                    ImDrawList* draw_list = ImGui::GetWindowDrawList();
                    
                    // Arrow parameters
                    float arrow_length = gizmo_size * 0.4f;
                    float arrow_head_size = 8.0f;
                    float line_thickness = 3.0f;
                    
                    // Helper lambda to draw arrow
                    auto DrawArrow = [&](ImVec2 start, ImVec2 end, ImU32 color, const char* label) {
                        // Draw arrow line
                        draw_list->AddLine(start, end, color, line_thickness);
                        
                        // Calculate arrow head direction
                        ImVec2 dir = ImVec2(end.x - start.x, end.y - start.y);
                        float len = sqrtf(dir.x * dir.x + dir.y * dir.y);
                        if (len > 0) {
                            dir.x /= len;
                            dir.y /= len;
                        }
                        
                        // Calculate perpendicular vector for arrow head
                        ImVec2 perp = ImVec2(-dir.y, dir.x);
                        
                        // Draw arrow head (triangle)
                        ImVec2 p1 = end;
                        ImVec2 p2 = ImVec2(end.x - dir.x * arrow_head_size - perp.x * arrow_head_size * 0.5f,
                                          end.y - dir.y * arrow_head_size - perp.y * arrow_head_size * 0.5f);
                        ImVec2 p3 = ImVec2(end.x - dir.x * arrow_head_size + perp.x * arrow_head_size * 0.5f,
                                          end.y - dir.y * arrow_head_size + perp.y * arrow_head_size * 0.5f);
                        draw_list->AddTriangleFilled(p1, p2, p3, color);
                        
                        // Draw label
                        ImVec2 label_pos = ImVec2(end.x + dir.x * 10.0f - 4.0f, end.y + dir.y * 10.0f - 7.0f);
                        draw_list->AddText(label_pos, color, label);
                    };
                    
                    // Get view matrix from Editor
                    const float* view_matrix = m_view_matrix;
                    
                    // Extract rotation part from view matrix (upper-left 3x3)
                    // View matrix transforms from world to camera space
                    // We need to extract the world axes in camera space
                    float3 x_axis_world = float3(view_matrix[0], view_matrix[4], view_matrix[8]);
                    float3 y_axis_world = float3(view_matrix[1], view_matrix[5], view_matrix[9]);
                    float3 z_axis_world = float3(view_matrix[2], view_matrix[6], view_matrix[10]);
                    
                    // Normalize axes
                    x_axis_world = Math::normalize(x_axis_world);
                    y_axis_world = Math::normalize(y_axis_world);
                    z_axis_world = Math::normalize(z_axis_world);
                    
                    // Project 3D axes to 2D screen space
                    // X axis points right in world space
                    ImVec2 x_end = ImVec2(gizmo_center.x + x_axis_world.x * arrow_length, 
                                         gizmo_center.y - x_axis_world.y * arrow_length);
                    
                    // Y axis points up in world space
                    ImVec2 y_end = ImVec2(gizmo_center.x + y_axis_world.x * arrow_length, 
                                         gizmo_center.y - y_axis_world.y * arrow_length);
                    
                    // Z axis points forward in world space
                    ImVec2 z_end = ImVec2(gizmo_center.x + z_axis_world.x * arrow_length, 
                                         gizmo_center.y - z_axis_world.y * arrow_length);
                    
                    // Draw origin sphere
                    draw_list->AddCircleFilled(gizmo_center, 5.0f, IM_COL32(255, 255, 255, 255));
                    draw_list->AddCircle(gizmo_center, 5.0f, IM_COL32(0, 0, 0, 255), 0, 1.5f);
                    
                    // Draw axes with proper depth ordering (draw farthest axis first)
                    // Check z component to determine drawing order
                    struct AxisInfo {
                        float depth;
                        ImVec2 end;
                        ImU32 color;
                        const char* label;
                    };
                    
                    AxisInfo axes[3] = {
                        { z_axis_world.z, z_end, IM_COL32(0, 0, 255, 255), "Z" },
                        { y_axis_world.z, y_end, IM_COL32(0, 255, 0, 255), "Y" },
                        { x_axis_world.z, x_end, IM_COL32(255, 0, 0, 255), "X" }
                    };
                    
                    // Sort axes by depth (draw back to front)
                    for (int i = 0; i < 2; i++) {
                        for (int j = i + 1; j < 3; j++) {
                            if (axes[i].depth > axes[j].depth) {
                                AxisInfo temp = axes[i];
                                axes[i] = axes[j];
                                axes[j] = temp;
                            }
                        }
                    }
                    
                    // Draw axes in sorted order
                    for (int i = 0; i < 3; i++) {
                        DrawArrow(gizmo_center, axes[i].end, axes[i].color, axes[i].label);
                    }
                }
                ImGui::EndChild();
            }
            ImGui::End();

            // Log window
            ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
            ImGui::Begin("Log");
            if (ImGui::SmallButton("[Test] Add test log entries"))
            {
                CB_INFO("Test info log message");
                CB_WARN("Test warning log message");
                CB_ERROR("Test error log message");
                CB_CORE_INFO("Test core info message");
                CB_CORE_WARN("Test core warning message");
            }
            ImGui::End();
            // Actually call in the regular Log helper (which will Begin() into the same window as we just did)
            log.Draw("Log");

            // Bottom: resource browser; Right: details panel
            if (m_show_content_browser)
                draw_content_browser();
            if (m_show_details_panel)
                draw_details_panel();
            if (m_show_scene_hierarchy)
                draw_scene_hierarchy();

            draw_save_as_popup();
            draw_content_browser_rename_popup();

            //ImGui::ShowDemoWindow(&show_demo_window);
        }

        namespace
        {
            // Directories we never surface in the content browser tree.
            bool is_hidden_directory(const std::string& name)
            {
                static const char* kBlacklist[] = {
                    ".git", ".vs", ".vscode", ".xmake", ".idea",
                    "build", "node_modules", "__pycache__"
                };
                for (const char* entry : kBlacklist)
                {
                    if (name == entry)
                        return true;
                }
                // Also hide any dot-prefixed directory (e.g. `.cache`).
                return !name.empty() && name.front() == '.';
            }

            std::string lowercase(std::string_view s)
            {
                std::string out(s);
                std::transform(out.begin(), out.end(), out.begin(),
                    [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
                return out;
            }

            bool path_is_inside_or_equal(const std::filesystem::path& child,
                                         const std::filesystem::path& parent)
            {
                std::filesystem::path rel = child.lexically_normal().lexically_relative(parent.lexically_normal());
                if (rel.empty())
                    return child.lexically_normal() == parent.lexically_normal();

                for (const auto& part : rel)
                {
                    if (part == "..")
                        return false;
                }
                return true;
            }

            bool first_relative_path_component(const std::filesystem::path& rel,
                                               std::filesystem::path& out_component)
            {
                for (const auto& part : rel)
                {
                    if (part == ".")
                        continue;
                    if (part == "..")
                        return false;
                    out_component = part;
                    return !out_component.empty();
                }
                return false;
            }

            bool is_invalid_rename_name(std::string_view name)
            {
                if (name.empty() || name == "." || name == "..")
                    return true;
                return name.find('/') != std::string_view::npos ||
                       name.find('\\') != std::string_view::npos;
            }

            std::filesystem::path make_unique_child_path(const std::filesystem::path& desired)
            {
                namespace fs = std::filesystem;

                std::error_code ec;
                if (!fs::exists(desired, ec))
                    return desired;

                const fs::path parent = desired.parent_path();
                const std::string stem = desired.stem().string();
                const std::string extension = desired.extension().string();
                for (uint32_t index = 1; index < 10000; ++index)
                {
                    fs::path candidate = parent / (stem + "_" + std::to_string(index) + extension);
                    ec.clear();
                    if (!fs::exists(candidate, ec))
                        return candidate;
                }

                return desired;
            }

        }

        std::filesystem::path Editor::resolve_content_browser_root() const
        {
            namespace fs = std::filesystem;

            fs::path project_root = fs::path(Core::FileHelper::get_project_root()).lexically_normal();
            if (project_root.empty())
                return {};

            Core::Application* app = m_pApp ? m_pApp : Core::Application::getApp();
            Samples::SampleApp* sample_app = app ? app->get_sample_app() : nullptr;
            RefCntAutoPtr<World> world = sample_app ? sample_app->get_world() : RefCntAutoPtr<World>{};

            if (world && !world->source_path().empty())
            {
                fs::path scene_path(world->source_path().c_str());
                if (!scene_path.is_absolute())
                    scene_path = project_root / scene_path;
                scene_path = scene_path.lexically_normal();

                const fs::path samples_root = (project_root / "samples").lexically_normal();
                if (path_is_inside_or_equal(scene_path, samples_root))
                {
                    fs::path rel = scene_path.lexically_relative(samples_root);
                    fs::path sample_name;
                    if (first_relative_path_component(rel, sample_name))
                        return (samples_root / sample_name / "content").lexically_normal();
                }

                const fs::path scene_dir = scene_path.parent_path();
                const std::string scene_dir_name = lowercase(scene_dir.filename().string());
                if (scene_dir_name == "content")
                    return scene_dir.lexically_normal();
                if (scene_dir_name == "assets" && !scene_dir.parent_path().empty())
                    return (scene_dir.parent_path() / "content").lexically_normal();
                if (!scene_dir.empty())
                    return (scene_dir / "content").lexically_normal();
            }

            const fs::path samples_root = (project_root / "samples").lexically_normal();
            std::error_code ec;
            if (fs::exists(samples_root, ec) && fs::is_directory(samples_root, ec))
                return samples_root;
            return project_root;
        }

        std::filesystem::path Editor::resolve_engine_content_root() const
        {
            namespace fs = std::filesystem;

            fs::path project_root = fs::path(Core::FileHelper::get_project_root()).lexically_normal();
            if (project_root.empty())
                return {};

            fs::path engine_content = (project_root / "Engine" / "content").lexically_normal();
            std::error_code ec;
            if (fs::exists(engine_content, ec) && fs::is_directory(engine_content, ec))
                return engine_content;
            return {};
        }

        bool Editor::is_content_browser_path_visible(const std::filesystem::path& path) const
        {
            if (path.empty())
                return false;

            std::filesystem::path normalized = path.lexically_normal();
            if (!m_tree_root.empty() &&
                path_is_inside_or_equal(normalized, std::filesystem::path(m_tree_root).lexically_normal()))
            {
                return true;
            }

            if (!m_engine_content_root.empty() &&
                path_is_inside_or_equal(normalized, std::filesystem::path(m_engine_content_root).lexically_normal()))
            {
                return true;
            }

            return false;
        }

        void Editor::refresh_content_browser_root(bool force)
        {
            namespace fs = std::filesystem;

            fs::path desired_root = resolve_content_browser_root();
            if (desired_root.empty())
                return;

            std::error_code ec;
            fs::create_directories(desired_root, ec);
            if (ec)
            {
                CB_WARN("Failed to create content folder: {} ({})",
                        desired_root.string().c_str(),
                        ec.message().c_str());
                return;
            }

            desired_root = desired_root.lexically_normal();
            const std::string desired_root_string = desired_root.string();
            const fs::path engine_root = resolve_engine_content_root();
            const std::string engine_root_string = engine_root.empty() ? std::string() : engine_root.string();

            Core::Application* app = m_pApp ? m_pApp : Core::Application::getApp();
            Samples::SampleApp* sample_app = app ? app->get_sample_app() : nullptr;
            RefCntAutoPtr<World> world = sample_app ? sample_app->get_world() : RefCntAutoPtr<World>{};
            const std::string scene_source = (world && !world->source_path().empty())
                ? std::string(world->source_path().c_str())
                : std::string();

            const bool root_changed = m_tree_root != desired_root_string;
            const bool engine_root_changed = m_engine_content_root != engine_root_string;
            const bool scene_changed = m_content_browser_scene_source != scene_source;
            if (!force && !root_changed && !engine_root_changed && !scene_changed)
                return;

            const fs::path old_root = m_tree_root.empty() ? fs::path{} : fs::path(m_tree_root).lexically_normal();
            const fs::path current = m_current_folder.empty() ? fs::path{} : fs::path(m_current_folder).lexically_normal();
            const bool current_is_visible =
                !current.empty() &&
                (path_is_inside_or_equal(current, desired_root) ||
                 (!engine_root.empty() && path_is_inside_or_equal(current, engine_root)));
            const bool keep_current_folder =
                !force &&
                current_is_visible;

            m_tree_root = desired_root_string;
            m_engine_content_root = engine_root_string;
            m_content_browser_scene_source = scene_source;

            if (!keep_current_folder)
            {
                m_current_folder = desired_root_string;
                m_tree_pending_expand = desired_root_string;
            }

            if (!m_selected_asset.empty())
            {
                fs::path selected(m_selected_asset);
                if (!selected.is_absolute() && !old_root.empty())
                    selected = old_root / selected;
                selected = selected.lexically_normal();
                if (!is_content_browser_path_visible(selected))
                    m_selected_asset.clear();
            }
        }

        void Editor::open_content_browser_item_location(const std::filesystem::path& path) const
        {
#ifdef CYBER_RUNTIME_PLATFORM_WINDOWS
            namespace fs = std::filesystem;

            std::error_code ec;
            fs::path target = path.is_absolute() ? path : fs::absolute(path, ec);
            if (ec)
                target = path;
            target = target.lexically_normal();

            const std::wstring parameters = L"/select,\"" + target.wstring() + L"\"";
            HINSTANCE result = ShellExecuteW(
                m_hwnd,
                L"open",
                L"explorer.exe",
                parameters.c_str(),
                nullptr,
                SW_SHOWNORMAL);

            if (reinterpret_cast<INT_PTR>(result) <= 32)
            {
                CB_WARN("Failed to open file location: {}", target.string().c_str());
            }
#else
            CB_WARN("Open file location is only implemented on Windows for now");
#endif
        }

        void Editor::draw_content_browser_item_context_menu(const std::filesystem::path& path,
                                                            const char* label,
                                                            bool allow_rename,
                                                            bool focus_folder)
        {
            if (!ImGui::BeginPopupContextItem("##content_browser_item_context"))
                return;

            const std::string full = path.string();
            const std::string fallback_label =
                path.filename().string().empty() ? full : path.filename().string();

            m_selected_asset = full;
            m_selected_node_id = 0;
            m_selected_component_index = -1;
            if (focus_folder)
                m_current_folder = full;

            ImGui::TextUnformatted((label && label[0] != '\0') ? label : fallback_label.c_str());
            ImGui::Separator();
            if (allow_rename)
            {
                if (ImGui::MenuItem("Rename..."))
                    begin_content_browser_rename(path);
            }
            else
            {
                ImGui::MenuItem("Rename...", nullptr, false, false);
            }
            if (ImGui::MenuItem("Open File Location"))
                open_content_browser_item_location(path);
            ImGui::EndPopup();
        }

        void Editor::create_content_browser_folder()
        {
            namespace fs = std::filesystem;

            refresh_content_browser_root(false);

            std::error_code ec;
            fs::path project_root = m_tree_root.empty() ? resolve_content_browser_root() : fs::path(m_tree_root);
            fs::path engine_root = m_engine_content_root.empty() ? resolve_engine_content_root() : fs::path(m_engine_content_root);
            fs::path destination_dir = m_current_folder.empty() ? project_root : fs::path(m_current_folder);
            fs::path root = project_root;
            if (!engine_root.empty() && path_is_inside_or_equal(destination_dir.lexically_normal(), engine_root.lexically_normal()))
                root = engine_root;
            if (root.empty())
            {
                CB_WARN("Content Browser has no root folder for folder creation");
                return;
            }

            root = root.lexically_normal();
            fs::create_directories(root, ec);
            if (ec)
            {
                CB_WARN("Failed to create content root: {} ({})",
                        root.string().c_str(),
                        ec.message().c_str());
                return;
            }

            if (!destination_dir.is_absolute())
                destination_dir = root / destination_dir;
            destination_dir = destination_dir.lexically_normal();

            ec.clear();
            const bool valid_destination =
                fs::exists(destination_dir, ec) &&
                fs::is_directory(destination_dir, ec) &&
                path_is_inside_or_equal(destination_dir, root);
            if (!valid_destination)
                destination_dir = root;

            const fs::path folder_path = make_unique_child_path(destination_dir / "New Folder");
            ec.clear();
            const bool created = fs::create_directory(folder_path, ec);
            if (ec || !created)
            {
                CB_WARN("Failed to create folder: {} ({})",
                        folder_path.string().c_str(),
                        ec ? ec.message().c_str() : "already exists");
                return;
            }

            m_current_folder = destination_dir.string();
            m_tree_pending_expand = m_current_folder;
            m_selected_asset = folder_path.string();
            m_selected_node_id = 0;
            m_selected_component_index = -1;
            m_show_content_browser = true;
            m_show_details_panel = true;

            CB_INFO("Created folder: {}", folder_path.string().c_str());
        }

        void Editor::begin_content_browser_rename(const std::filesystem::path& path)
        {
            m_rename_target_path = path.string();
            std::snprintf(m_rename_buffer, sizeof(m_rename_buffer), "%s",
                          path.filename().string().c_str());
            m_show_rename_popup = true;

            m_selected_asset = m_rename_target_path;
            m_selected_node_id = 0;
            m_selected_component_index = -1;
        }

        void Editor::draw_content_browser_rename_popup()
        {
            namespace fs = std::filesystem;

            if (m_show_rename_popup)
            {
                ImGui::OpenPopup("Rename");
                m_show_rename_popup = false;
            }

            ImGui::SetNextWindowSize(ImVec2(420, 0), ImGuiCond_Appearing);
            if (ImGui::BeginPopupModal("Rename", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
            {
                fs::path old_path(m_rename_target_path);
                ImGui::TextUnformatted("New name:");
                ImGui::SetNextItemWidth(360.0f);
                if (ImGui::IsWindowAppearing())
                    ImGui::SetKeyboardFocusHere();
                const bool submit_from_input = ImGui::InputText(
                    "##rename_name",
                    m_rename_buffer,
                    sizeof(m_rename_buffer),
                    ImGuiInputTextFlags_EnterReturnsTrue);

                bool should_submit = submit_from_input;
                if (ImGui::Button("Rename", ImVec2(120, 0)))
                    should_submit = true;
                ImGui::SameLine();
                if (ImGui::Button("Cancel", ImVec2(120, 0)))
                    ImGui::CloseCurrentPopup();

                if (should_submit)
                {
                    const std::string new_name = m_rename_buffer;
                    if (is_invalid_rename_name(new_name))
                    {
                        CB_WARN("Invalid rename target: {}", new_name.c_str());
                    }
                    else
                    {
                        std::error_code ec;
                        fs::path normalized_old = old_path.lexically_normal();
                        fs::path new_path = (normalized_old.parent_path() / new_name).lexically_normal();

                        if (normalized_old == new_path)
                        {
                            ImGui::CloseCurrentPopup();
                        }
                        else if (fs::exists(new_path, ec))
                        {
                            CB_WARN("Rename target already exists: {}", new_path.string().c_str());
                        }
                        else
                        {
                            ec.clear();
                            fs::rename(normalized_old, new_path, ec);
                            if (ec)
                            {
                                CB_WARN("Failed to rename: {} -> {} ({})",
                                        normalized_old.string().c_str(),
                                        new_path.string().c_str(),
                                        ec.message().c_str());
                            }
                            else
                            {
                                auto rewrite_if_inside = [&](std::string& stored_path)
                                {
                                    if (stored_path.empty())
                                        return;
                                    fs::path current(stored_path);
                                    if (!current.is_absolute())
                                        return;
                                    current = current.lexically_normal();
                                    if (!path_is_inside_or_equal(current, normalized_old))
                                        return;

                                    fs::path rel = current.lexically_relative(normalized_old);
                                    stored_path = (new_path / rel).lexically_normal().string();
                                };

                                rewrite_if_inside(m_current_folder);
                                rewrite_if_inside(m_selected_asset);
                                m_selected_asset = new_path.string();
                                m_tree_pending_expand = new_path.parent_path().string();
                                m_texture_info_asset.clear();
                                CB_INFO("Renamed: {} -> {}",
                                        normalized_old.string().c_str(),
                                        new_path.string().c_str());
                                ImGui::CloseCurrentPopup();
                            }
                        }
                    }
                }

                ImGui::EndPopup();
            }
        }

        void Editor::load_content_browser_icons(RenderObject::IRenderDevice* device)
        {
            if (m_content_browser_icons_loaded || !device)
                return;

#ifdef CYBER_RUNTIME_PLATFORM_WINDOWS
            const std::filesystem::path icon_root =
                std::filesystem::path(Core::FileHelper::get_project_root()) /
                "Engine/Resources/Editor/Icons/individual";

            auto load_icon = [&](ContentBrowserIcon icon, const char* filename)
            {
                std::vector<uint8_t> pixels;
                uint32_t width = 0;
                uint32_t height = 0;
                const std::filesystem::path path = icon_root / filename;
                if (!load_png_rgba_with_wic(path, pixels, width, height))
                    return;

                RenderObject::TextureCreateDesc texture_desc = {};
                texture_desc.m_name = CYBER_UTF8("EditorContentBrowserIcon");
                texture_desc.m_width = width;
                texture_desc.m_height = height;
                texture_desc.m_depth = 1;
                texture_desc.m_arraySize = 1;
                texture_desc.m_mipLevels = 1;
                texture_desc.m_dimension = TEX_DIMENSION_2D;
                texture_desc.m_format = TEX_FORMAT_RGBA8_UNORM;
                texture_desc.m_bindFlags = GRAPHICS_RESOURCE_BIND_SHADER_RESOURCE;
                texture_desc.m_initializeState = GRAPHICS_RESOURCE_STATE_SHADER_RESOURCE;
                texture_desc.m_sampleCount = SAMPLE_COUNT_1;
                texture_desc.m_sampleQuality = 1;

                RenderObject::TextureSubResData sub_res_data = {};
                sub_res_data.pData = pixels.data();
                sub_res_data.stride = static_cast<uint64_t>(width) * 4ull;
                sub_res_data.depthStride = static_cast<uint64_t>(width) * static_cast<uint64_t>(height) * 4ull;
                sub_res_data.srcOffset = 0;

                RenderObject::TextureData texture_data = {};
                texture_data.pSubResources = &sub_res_data;
                texture_data.numSubResources = 1;
                texture_data.pDevice = device;

                RenderObject::ITexture* texture = nullptr;
                device->create_texture(texture_desc, &texture_data, &texture);
                if (!texture)
                {
                    CB_WARN("Failed to create editor icon texture: {}", path.string().c_str());
                    return;
                }

                auto& slot = m_content_browser_icons[static_cast<size_t>(icon)];
                slot.texture.attach(texture);
                slot.view = texture->get_default_texture_view(TEXTURE_VIEW_SHADER_RESOURCE);
            };

            load_icon(ContentBrowserIcon::Folder,  "asset_icon_01_folder.png");
            load_icon(ContentBrowserIcon::Scene,   "asset_icon_02_scene.png");
            load_icon(ContentBrowserIcon::Model,   "asset_icon_03_mesh.png");
            load_icon(ContentBrowserIcon::Texture, "asset_icon_05_texture.png");
            load_icon(ContentBrowserIcon::Shader,  "asset_icon_06_shader.png");
            load_icon(ContentBrowserIcon::File,    "asset_icon_07_script.png");
#endif

            m_content_browser_icons_loaded = true;
        }

        RenderObject::ITexture_View* Editor::content_browser_icon_view(bool is_dir, const ResourceTypeInfo* type_info) const
        {
            ContentBrowserIcon icon = ContentBrowserIcon::File;
            if (is_dir)
            {
                icon = ContentBrowserIcon::Folder;
            }
            else if (type_info)
            {
                switch (type_info->category)
                {
                    case ResourceCategory::Model:   icon = ContentBrowserIcon::Model; break;
                    case ResourceCategory::Texture: icon = ContentBrowserIcon::Texture; break;
                    case ResourceCategory::Shader:  icon = ContentBrowserIcon::Shader; break;
                    case ResourceCategory::Scene:   icon = ContentBrowserIcon::Scene; break;
                    default:                        icon = ContentBrowserIcon::File; break;
                }
            }

            return m_content_browser_icons[static_cast<size_t>(icon)].view;
        }

        void Editor::draw_directory_tree(const std::filesystem::path& dir, int depth)
        {
            namespace fs = std::filesystem;
            std::error_code ec;

            std::string label = (depth == 0)
                ? dir.filename().string().empty() ? dir.string() : dir.filename().string()
                : dir.filename().string();
            if (label.empty())
                label = dir.string();

            const std::string dir_str = dir.string();
            if (depth == 0)
            {
                const fs::path normalized_dir = dir.lexically_normal();
                if (!m_engine_content_root.empty() &&
                    normalized_dir == fs::path(m_engine_content_root).lexically_normal())
                {
                    label = "Engine Content";
                }
                else if (!m_tree_root.empty() &&
                         normalized_dir == fs::path(m_tree_root).lexically_normal())
                {
                    label = "Project Content";
                }
            }

            const bool selected = (dir_str == m_current_folder);

            ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow
                                     | ImGuiTreeNodeFlags_OpenOnDoubleClick
                                     | ImGuiTreeNodeFlags_SpanFullWidth;
            if (selected)
                flags |= ImGuiTreeNodeFlags_Selected;
            if (depth == 0)
                flags |= ImGuiTreeNodeFlags_DefaultOpen;

            // Force this node open if a pending "expand-to" target is under it.
            if (!m_tree_pending_expand.empty())
            {
                if (m_tree_pending_expand.rfind(dir_str, 0) == 0)
                    ImGui::SetNextItemOpen(true, ImGuiCond_Always);
            }

            ImGui::PushID(dir_str.c_str());
            const bool open = ImGui::TreeNodeEx("##node", flags, "%s", label.c_str());

            if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
            {
                m_current_folder = dir_str;
            }
            draw_content_browser_item_context_menu(dir, label.c_str(), depth > 0, true);

            if (open)
            {
                for (const auto& entry : fs::directory_iterator(dir, fs::directory_options::skip_permission_denied, ec))
                {
                    std::error_code is_dir_ec;
                    if (!entry.is_directory(is_dir_ec))
                        continue;
                    const std::string name = entry.path().filename().string();
                    if (is_hidden_directory(name))
                        continue;
                    draw_directory_tree(entry.path(), depth + 1);
                }
                ImGui::TreePop();
            }
            ImGui::PopID();
        }

        void Editor::draw_content_browser()
        {
            if (!ImGui::Begin("Content Browser", &m_show_content_browser))
            {
                ImGui::End();
                return;
            }

            namespace fs = std::filesystem;
            std::error_code ec;

            refresh_content_browser_root(false);

            // Make sure the tree root + current folder are valid.
            if (m_tree_root.empty() || !fs::exists(m_tree_root, ec) || !fs::is_directory(m_tree_root, ec))
            {
                refresh_content_browser_root(true);
            }
            fs::path current_folder_path = m_current_folder.empty() ? fs::path{} : fs::path(m_current_folder).lexically_normal();
            if (m_current_folder.empty() ||
                !fs::exists(m_current_folder, ec) ||
                !fs::is_directory(m_current_folder, ec) ||
                !is_content_browser_path_visible(current_folder_path))
            {
                m_current_folder = m_tree_root;
            }

            if (ImGui::BeginTable("##cb_split", 2,
                ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_NoSavedSettings))
            {
                ImGui::TableSetupColumn("##tree", ImGuiTableColumnFlags_WidthFixed, m_tree_pane_width);
                ImGui::TableSetupColumn("##grid", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableNextRow();

                // ---- Left pane: directory tree ----
                ImGui::TableSetColumnIndex(0);
                if (ImGui::BeginChild("##cb_tree", ImVec2(0, 0), ImGuiChildFlags_None))
                {
                    if (!m_tree_root.empty())
                        draw_directory_tree(fs::path(m_tree_root), 0);
                    if (!m_engine_content_root.empty() && m_engine_content_root != m_tree_root)
                        draw_directory_tree(fs::path(m_engine_content_root), 0);
                }
                ImGui::EndChild();

                // Consume the pending-expand target after the tree has been drawn.
                m_tree_pending_expand.clear();

                // ---- Right pane: filtered tile grid ----
                ImGui::TableSetColumnIndex(1);
                ImGui::TextUnformatted(m_current_folder.c_str());
                ImGui::Separator();

                const float thumb_size = 72.0f;
                const float cell_padding = 16.0f;
                const float cell_size = thumb_size + cell_padding;
                float panel_width = ImGui::GetContentRegionAvail().x;
                int columns = (int)(panel_width / cell_size);
                if (columns < 1) columns = 1;

                ImGui::BeginChild("##cb_grid", ImVec2(0, 0), ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar);
                ImGui::Columns(columns, nullptr, false);

                const auto& registry = ResourceTypeRegistry::get();

                auto draw_tile = [&](const fs::directory_entry& entry, bool is_dir, const ResourceTypeInfo* type_info)
                {
                    const std::string name = entry.path().filename().string();
                    const std::string full = entry.path().string();

                    ImGui::PushID(full.c_str());
                    const bool selected = (m_selected_asset == full);

                    ImVec2 cursor_before = ImGui::GetCursorScreenPos();
                    if (ImGui::Selectable("##tile", selected,
                        ImGuiSelectableFlags_AllowDoubleClick, ImVec2(thumb_size, thumb_size)))
                    {
                        m_selected_asset = full;
                        m_selected_node_id = 0;
                        m_selected_component_index = -1;
                        if (is_dir && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                        {
                            m_current_folder = full;
                            m_tree_pending_expand = full;
                        }
                    }

                    draw_content_browser_item_context_menu(entry.path(), name.c_str(), true, false);

                    // Allow dragging files (not directories) onto other panels.
                    if (!is_dir && ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
                    {
                        ImGui::SetDragDropPayload("CYBER_ASSET", full.c_str(), full.size() + 1);
                        ImGui::TextUnformatted(name.c_str());
                        ImGui::EndDragDropSource();
                    }

                    // Category tint bar under the tile.
                    ImU32 bar_color = IM_COL32(120, 120, 120, 255);
                    const char* glyph = is_dir ? "[DIR]" : "[FILE]";
                    if (is_dir)
                    {
                        bar_color = IM_COL32(230, 200, 120, 255);
                    }
                    else if (type_info)
                    {
                        bar_color = type_info->tint_color;
                        switch (type_info->category)
                        {
                            case ResourceCategory::Model:   glyph = "MDL"; break;
                            case ResourceCategory::Texture: glyph = "TEX"; break;
                            case ResourceCategory::Shader:  glyph = "SHD"; break;
                            case ResourceCategory::Scene:   glyph = "SCN"; break;
                            default: break;
                        }
                    }

                    ImDrawList* dl = ImGui::GetWindowDrawList();
                    RenderObject::ITexture_View* icon_view = content_browser_icon_view(is_dir, type_info);
                    if (icon_view)
                    {
                        const float icon_padding = 7.0f;
                        ImVec2 icon_min = ImVec2(cursor_before.x + icon_padding, cursor_before.y + icon_padding);
                        ImVec2 icon_max = ImVec2(cursor_before.x + thumb_size - icon_padding,
                                                 cursor_before.y + thumb_size - icon_padding);
                        dl->AddImage((ImTextureID)icon_view, icon_min, icon_max);
                    }
                    else
                    {
                        // Fallback for missing icon assets or failed texture creation.
                        ImVec2 text_size = ImGui::CalcTextSize(glyph);
                        ImVec2 text_pos = ImVec2(
                            cursor_before.x + (thumb_size - text_size.x) * 0.5f,
                            cursor_before.y + (thumb_size - text_size.y) * 0.5f - 4.0f);
                        dl->AddText(text_pos, IM_COL32(230, 230, 230, 255), glyph);
                    }

                    ImVec2 bar_min = ImVec2(cursor_before.x, cursor_before.y + thumb_size - 6.0f);
                    ImVec2 bar_max = ImVec2(cursor_before.x + thumb_size, cursor_before.y + thumb_size);
                    dl->AddRectFilled(bar_min, bar_max, bar_color);

                    ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + thumb_size);
                    ImGui::TextWrapped("%s", name.c_str());
                    ImGui::PopTextWrapPos();

                    ImGui::NextColumn();
                    ImGui::PopID();
                };

                // Directories first.
                for (const auto& entry : fs::directory_iterator(m_current_folder,
                    fs::directory_options::skip_permission_denied, ec))
                {
                    std::error_code is_dir_ec;
                    if (!entry.is_directory(is_dir_ec))
                        continue;
                    if (is_hidden_directory(entry.path().filename().string()))
                        continue;
                    draw_tile(entry, true, nullptr);
                }

                // Then registered-type files only.
                for (const auto& entry : fs::directory_iterator(m_current_folder,
                    fs::directory_options::skip_permission_denied, ec))
                {
                    std::error_code is_dir_ec;
                    if (entry.is_directory(is_dir_ec))
                        continue;
                    const std::string ext = lowercase(entry.path().extension().string());
                    const ResourceTypeInfo* info = registry.find(ext);
                    if (!info)
                        continue;
                    draw_tile(entry, false, info);
                }

                ImGui::Columns(1);
                if (ImGui::BeginPopupContextWindow(
                    "##cb_grid_context",
                    ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
                {
                    if (ImGui::BeginMenu("Create"))
                    {
                        if (ImGui::MenuItem("Folder"))
                            create_content_browser_folder();
                        ImGui::EndMenu();
                    }
                    ImGui::EndPopup();
                }
                ImGui::EndChild();

                ImGui::EndTable();
            }

            ImGui::End();
        }

        void Editor::draw_details_panel()
        {
            if (!ImGui::Begin("Details", &m_show_details_panel))
            {
                ImGui::End();
                return;
            }

            // Scene selection takes precedence over Content Browser selection.
            if (m_selected_node_id != 0)
            {
                Core::Application* app = m_pApp ? m_pApp : Core::Application::getApp();
                Samples::SampleApp* sample_app = app ? app->get_sample_app() : nullptr;
                RefCntAutoPtr<World> world = sample_app ? sample_app->get_world() : RefCntAutoPtr<World>{};
                SceneNode* node = world ? world->find_node(m_selected_node_id) : nullptr;

                if (!node)
                {
                    ImGui::TextDisabled("(selected node no longer exists)");
                    ImGui::End();
                    return;
                }

                // Node-level (SceneNode fields aren't driven by the property
                // registry — only Primitive-derived components are).
                {
                    char name_buf[128];
                    std::snprintf(name_buf, sizeof(name_buf), "%s", node->name.c_str());
                    if (ImGui::InputText("Node Name", name_buf, sizeof(name_buf)))
                    {
                        node->name = name_buf;
                        world->set_dirty(true);
                    }
                }
                ImGui::Text("ID: %u", node->id);
                ImGui::Text("Components: %zu", (size_t)node->components.size());
                ImGui::Separator();

                Component::Primitive* comp = nullptr;
                if (m_selected_component_index >= 0 &&
                    (size_t)m_selected_component_index < node->components.size())
                {
                    comp = node->components[(size_t)m_selected_component_index].get();
                }

                if (!comp)
                {
                    ImGui::TextWrapped("Select a component in the hierarchy to see its properties.");
                    ImGui::End();
                    return;
                }

                ImGui::Text("Type: %s", comp->type_name());
                if (draw_component_properties(comp, comp->type_name(), m_property_draw_ctx))
                    world->set_dirty(true);

                // Runtime / derived-value panels — these aren't simple
                // editable fields so they stay hand-written.
                if (auto* mc = dynamic_cast<Component::MeshComponent*>(comp))
                {
                    ImGui::Separator();
                    ImGui::TextDisabled("Mesh Runtime");
                    if (mc->mesh)
                        ImGui::Text("Vertices / Indices: %u / %u",
                                    mc->mesh->vertex_count, mc->mesh->index_count);
                    else if (!mc->model_resource.empty())
                        ImGui::TextDisabled("(mesh loading...)");
                    ImGui::Text("GPU ready: %s", mc->gpu_ready ? "true" : "false");
                }
                else if (auto* cc = dynamic_cast<Component::CameraComponent*>(comp))
                {
                    ImGui::Separator();
                    ImGui::TextDisabled("Camera Runtime");
                    ImGui::Text("Yaw  : %.3f", cc->get_yaw());
                    ImGui::Text("Pitch: %.3f", cc->get_pitch());
                }
                else if (auto* dl = dynamic_cast<Component::DirectionalLightComponent*>(comp))
                {
                    ImGui::Separator();
                    ImGui::TextDisabled("Light Runtime");
                    float3 dir = dl->get_direction();
                    ImGui::Text("Direction: %.3f, %.3f, %.3f", dir.x, dir.y, dir.z);
                }

                ImGui::End();
                return;
            }

            if (m_selected_asset.empty())
            {
                ImGui::TextDisabled("No selection");
                ImGui::Separator();
                ImGui::TextWrapped("Select an item in the Scene Hierarchy or Content Browser to see its details here.");
                ImGui::End();
                return;
            }

            namespace fs = std::filesystem;
            fs::path p(m_selected_asset);
            std::error_code ec;

            ImGui::TextUnformatted("Name:");
            ImGui::SameLine();
            ImGui::TextUnformatted(p.filename().string().c_str());

            ImGui::TextUnformatted("Path:");
            ImGui::SameLine();
            ImGui::PushTextWrapPos();
            ImGui::TextUnformatted(p.string().c_str());
            ImGui::PopTextWrapPos();

            const auto& registry = ResourceTypeRegistry::get();
            const bool exists = fs::exists(p, ec);
            const bool is_dir = exists && fs::is_directory(p, ec);

            if (!exists)
            {
                ImGui::TextDisabled("(not found on disk)");
            }
            else if (is_dir)
            {
                ImGui::TextUnformatted("Type: Folder");
            }
            else
            {
                const std::string ext = lowercase(p.extension().string());
                const ResourceTypeInfo* info = registry.find(ext);
                if (info)
                {
                    ImGui::Text("Type: %s", info->display_name.c_str());
                    ImGui::Text("Category: %s", registry.category_label(info->category));
                }
                else
                {
                    ImGui::Text("Type: %s", ext.empty() ? "(unknown)" : ext.c_str());
                    ImGui::TextDisabled("(not a registered resource type)");
                }

                auto size = fs::file_size(p, ec);
                if (!ec)
                    ImGui::Text("Size: %llu bytes", (unsigned long long)size);

                if (info && info->category == ResourceCategory::Texture)
                {
                    const std::string asset_path = p.string();
                    if (m_texture_info_asset != asset_path)
                    {
                        m_texture_info_asset = asset_path;
                        m_texture_info_width = 0;
                        m_texture_info_height = 0;
                        m_texture_info_valid = false;
                        m_texture_info_is_editor_asset = false;
                        m_texture_info_source_format.clear();
                        m_texture_info_guid.clear();
                        m_texture_info_content_hash = 0;
                        m_texture_info_payload_size = 0;

                        if (ext == ".textureasset")
                        {
                            TextureEditorAssetInfo texture_asset_info;
                            if (TextureImporter::ReadInfo(p, texture_asset_info))
                            {
                                m_texture_info_valid = true;
                                m_texture_info_is_editor_asset = true;
                                m_texture_info_width = texture_asset_info.payloadHeader.width;
                                m_texture_info_height = texture_asset_info.payloadHeader.height;
                                m_texture_info_source_format = texture_asset_info.sourceExtension;
                                m_texture_info_guid = texture_asset_info.fileHeader.assetGuid.ToString();
                                m_texture_info_content_hash = texture_asset_info.fileHeader.contentHash;
                                m_texture_info_payload_size = texture_asset_info.fileHeader.payloadSize;
                            }
                        }
                        else
                        {
#ifdef CYBER_RUNTIME_PLATFORM_WINDOWS
                            m_texture_info_valid = query_image_size_with_wic(
                                p, m_texture_info_width, m_texture_info_height);
#endif
                        }
                    }

                    if (m_texture_info_is_editor_asset)
                    {
                        ImGui::TextUnformatted("Asset:");
                        ImGui::SameLine();
                        ImGui::TextUnformatted(m_texture_info_guid.c_str());
                        ImGui::Text("Payload: %llu bytes", (unsigned long long)m_texture_info_payload_size);
                        ImGui::Text("Content Hash: 0x%016llx", (unsigned long long)m_texture_info_content_hash);
                        ImGui::Text("Source Format: %s",
                                    m_texture_info_source_format.empty() ? "(unknown)" : m_texture_info_source_format.c_str());
                    }

                    if (m_texture_info_valid)
                    {
                        ImGui::Text("Dimensions: %u x %u",
                                    m_texture_info_width,
                                    m_texture_info_height);
                    }
                    else
                    {
                        ImGui::TextDisabled("Dimensions: (unreadable)");
                    }
                }
            }

            ImGui::Separator();
            ImGui::TextDisabled("Properties");
            ImGui::BulletText("Inspector stub - extend per asset type");

            ImGui::End();
        }

        void Editor::draw_scene_hierarchy()
        {
            if (!ImGui::Begin("Scene Hierarchy", &m_show_scene_hierarchy))
            {
                ImGui::End();
                return;
            }

            Core::Application* app = m_pApp ? m_pApp : Core::Application::getApp();
            Samples::SampleApp* sample_app = app ? app->get_sample_app() : nullptr;
            RefCntAutoPtr<World> world = sample_app ? sample_app->get_world() : RefCntAutoPtr<World>{};

            if (!world)
            {
                ImGui::TextDisabled("No active world");
                ImGui::End();
                return;
            }

            if (!world->source_path().empty())
                ImGui::Text("Scene: %s%s", world->source_path().c_str(), world->is_dirty() ? " *" : "");
            else
                ImGui::TextDisabled("Scene: (unsaved)%s", world->is_dirty() ? " *" : "");

            const auto& nodes = world->get_nodes();
            ImGui::Text("Objects: %zu", (size_t)nodes.size());
            ImGui::Separator();

            ImGui::BeginChild("##hierarchy_list", ImVec2(0, 0), ImGuiChildFlags_None);

            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CYBER_ASSET"))
                {
                    const char* path = static_cast<const char*>(payload->Data);
                    try_accept_asset_drop(path, float3{0.0f, 0.0f, 0.0f});
                }
                ImGui::EndDragDropTarget();
            }

            uint32_t node_to_delete = 0;
            uint32_t node_to_duplicate = 0;
            uint32_t comp_delete_node = 0;
            int      comp_delete_index = -1;

            for (size_t i = 0; i < nodes.size(); ++i)
            {
                const SceneNode& node = nodes[i];
                ImGui::PushID((int)node.id);

                const char* display_name = node.name.empty() ? "(unnamed)" : node.name.c_str();

                ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow
                                              | ImGuiTreeNodeFlags_OpenOnDoubleClick
                                              | ImGuiTreeNodeFlags_SpanFullWidth
                                              | ImGuiTreeNodeFlags_DefaultOpen;
                const bool node_selected = (node.id == m_selected_node_id && m_selected_component_index < 0);
                if (node_selected)
                    node_flags |= ImGuiTreeNodeFlags_Selected;

                bool node_open = ImGui::TreeNodeEx(display_name, node_flags);

                if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
                {
                    m_selected_node_id = node.id;
                    m_selected_component_index = -1;
                }

                if (ImGui::BeginPopupContextItem())
                {
                    m_selected_node_id = node.id;
                    m_selected_component_index = -1;
                    if (ImGui::MenuItem("Duplicate"))
                        node_to_duplicate = node.id;
                    if (ImGui::MenuItem("Delete"))
                        node_to_delete = node.id;
                    ImGui::EndPopup();
                }

                if (node_open)
                {
                    for (size_t c = 0; c < node.components.size(); ++c)
                    {
                        const auto* comp = node.components[c].get();
                        if (!comp)
                            continue;

                        ImGui::PushID((int)(c + 1));

                        const char* cname = comp->name.empty() ? comp->type_name() : comp->name.c_str();
                        const bool comp_selected = (node.id == m_selected_node_id &&
                                                    (int)c == m_selected_component_index);
                        if (ImGui::Selectable(cname, comp_selected))
                        {
                            m_selected_node_id = node.id;
                            m_selected_component_index = (int)c;
                        }

                        if (ImGui::BeginPopupContextItem())
                        {
                            m_selected_node_id = node.id;
                            m_selected_component_index = (int)c;
                            if (ImGui::MenuItem("Delete Component"))
                            {
                                comp_delete_node = node.id;
                                comp_delete_index = (int)c;
                            }
                            ImGui::EndPopup();
                        }

                        ImGui::PopID();
                    }
                    ImGui::TreePop();
                }
                ImGui::PopID();
            }

            ImGui::EndChild();

            // Deferred mutations.
            if (node_to_delete != 0)
            {
                world->remove_node(node_to_delete);
                if (m_selected_node_id == node_to_delete)
                {
                    m_selected_node_id = 0;
                    m_selected_component_index = -1;
                }
            }
            if (node_to_duplicate != 0)
            {
                if (const SceneNode* src = world->find_node(node_to_duplicate))
                {
                    SceneNode copy = src->deep_clone();
                    // Nudge every Primitive in the clone so it's visually offset.
                    for (auto& comp : copy.components)
                        if (comp) comp->position.x += 1.0f;

                    uint32_t new_id = world->add_node(std::move(copy));

                    // Re-enqueue any MeshComponent reloads — GPU state was reset
                    // by the clone pass.
                    if (SceneNode* added = world->find_node(new_id))
                    {
                        for (uint32_t idx = 0; idx < added->components.size(); ++idx)
                        {
                            auto* mc = dynamic_cast<Component::MeshComponent*>(added->components[idx].get());
                            if (mc && !mc->model_resource.empty())
                                world->enqueue_pending_load(new_id, idx, mc->model_resource);
                        }
                    }
                    m_selected_node_id = new_id;
                    m_selected_component_index = -1;
                }
            }
            if (comp_delete_node != 0 && comp_delete_index >= 0)
            {
                if (SceneNode* n = world->find_node(comp_delete_node))
                {
                    if ((size_t)comp_delete_index < n->components.size())
                    {
                        n->components.erase(n->components.begin() + comp_delete_index);
                        world->set_dirty(true);
                        if (m_selected_node_id == comp_delete_node &&
                            m_selected_component_index == comp_delete_index)
                        {
                            m_selected_component_index = -1;
                        }
                    }
                }
            }

            ImGui::End();
        }

        void Editor::handle_import_image()
        {
#ifdef CYBER_RUNTIME_PLATFORM_WINDOWS
            namespace fs = std::filesystem;

            refresh_content_browser_root(false);

            fs::path source_path;
            if (!open_image_import_dialog(m_hwnd, source_path))
                return;

            std::error_code ec;
            if (!fs::exists(source_path, ec) || fs::is_directory(source_path, ec))
            {
                CB_WARN("Image import source is not a file: {}", source_path.string().c_str());
                return;
            }

            const std::string ext = lowercase(source_path.extension().string());
            if (ext == ".textureasset")
            {
                CB_WARN("Image import source is already a texture asset: {}", source_path.string().c_str());
                return;
            }

            const ResourceTypeInfo* info = ResourceTypeRegistry::get().find(ext);
            if (!info || info->category != ResourceCategory::Texture ||
                !TextureImporter::IsSupportedSourceExtension(ext))
            {
                CB_WARN("Image import rejected unsupported type: {}", source_path.string().c_str());
                return;
            }

            fs::path destination_dir = m_current_folder.empty() ? fs::path{} : fs::path(m_current_folder);
            ec.clear();
            if (destination_dir.empty() || !fs::exists(destination_dir, ec) || !fs::is_directory(destination_dir, ec))
                destination_dir = m_tree_root.empty() ? fs::path{} : fs::path(m_tree_root);
            ec.clear();
            if (destination_dir.empty() || !fs::exists(destination_dir, ec) || !fs::is_directory(destination_dir, ec))
            {
                destination_dir = resolve_content_browser_root();
                fs::create_directories(destination_dir, ec);
            }
            if (ec || destination_dir.empty())
            {
                CB_WARN("Image import has no valid destination folder");
                return;
            }

            uint32_t width = 0;
            uint32_t height = 0;
            const bool has_dimensions = query_image_size_with_wic(source_path, width, height);

            fs::path content_root = m_tree_root.empty() ? resolve_content_browser_root() : fs::path(m_tree_root);
            fs::path engine_root = m_engine_content_root.empty() ? resolve_engine_content_root() : fs::path(m_engine_content_root);
            if (!engine_root.empty() &&
                path_is_inside_or_equal(destination_dir.lexically_normal(), engine_root.lexically_normal()))
            {
                content_root = engine_root;
            }
            if (content_root.empty())
                content_root = destination_dir;
            content_root = content_root.lexically_normal();

            AssetDatabase asset_database(content_root);
            if (!asset_database.Load())
            {
                CB_ERROR("Failed to load asset registry: {}", asset_database.RegistryPath().string().c_str());
                return;
            }

            const fs::path destination_path = make_unique_child_path(
                destination_dir / (source_path.stem().string() + ".textureasset"));

            AssetImportRequest import_request;
            import_request.sourcePath = source_path;
            import_request.destinationPath = destination_path;
            import_request.contentRoot = content_root;
            import_request.width = width;
            import_request.height = height;

            const std::string stored_asset_path = asset_database.MakeStoredPath(destination_path);
            if (const AssetRegistryRecord* existing = asset_database.Registry().FindByAssetPath(stored_asset_path))
                import_request.existingGuid = existing->guid;

            TextureImporter importer;
            AssetImportResult import_result;
            if (!importer.Import(import_request, import_result))
            {
                CB_ERROR("Failed to import texture asset: {} -> {} ({})",
                         source_path.string().c_str(),
                         destination_path.string().c_str(),
                         import_result.error.c_str());
                return;
            }

            asset_database.Registry().Upsert(import_result.registryRecord);
            if (!asset_database.Save())
            {
                CB_ERROR("Failed to save asset registry: {}", asset_database.RegistryPath().string().c_str());
                return;
            }

            m_current_folder = destination_dir.string();
            m_tree_pending_expand = m_current_folder;
            m_selected_asset = destination_path.string();
            m_selected_node_id = 0;
            m_selected_component_index = -1;
            m_show_content_browser = true;
            m_show_details_panel = true;

            if (has_dimensions)
            {
                CB_INFO("Imported image asset: {} ({}x{})",
                        destination_path.string().c_str(),
                        width,
                        height);
            }
            else
            {
                CB_INFO("Imported image asset: {} (metadata unreadable by WIC)",
                        destination_path.string().c_str());
            }
            CB_INFO("Updated asset registry: {}", asset_database.RegistryPath().string().c_str());
#else
            CB_WARN("Image import is only implemented on Windows for now");
#endif
        }

        void Editor::handle_import_mesh()
        {
#ifdef CYBER_RUNTIME_PLATFORM_WINDOWS
            namespace fs = std::filesystem;

            refresh_content_browser_root(false);

            fs::path source_path;
            if (!open_mesh_import_dialog(m_hwnd, source_path))
                return;

            std::error_code ec;
            if (!fs::exists(source_path, ec) || fs::is_directory(source_path, ec))
            {
                CB_WARN("Mesh import source is not a file: {}", source_path.string().c_str());
                return;
            }

            const std::string ext = lowercase(source_path.extension().string());
            if (ext == ".meshasset")
            {
                CB_WARN("Mesh import source is already a mesh asset: {}", source_path.string().c_str());
                return;
            }

            const ResourceTypeInfo* info = ResourceTypeRegistry::get().find(ext);
            if (!info || info->category != ResourceCategory::Model ||
                !MeshImporter::IsSupportedSourceExtension(ext))
            {
                CB_WARN("Mesh import rejected unsupported type: {}", source_path.string().c_str());
                return;
            }

            fs::path destination_dir = m_current_folder.empty() ? fs::path{} : fs::path(m_current_folder);
            ec.clear();
            if (destination_dir.empty() || !fs::exists(destination_dir, ec) || !fs::is_directory(destination_dir, ec))
                destination_dir = m_tree_root.empty() ? fs::path{} : fs::path(m_tree_root);
            ec.clear();
            if (destination_dir.empty() || !fs::exists(destination_dir, ec) || !fs::is_directory(destination_dir, ec))
            {
                destination_dir = resolve_content_browser_root();
                fs::create_directories(destination_dir, ec);
            }
            if (ec || destination_dir.empty())
            {
                CB_WARN("Mesh import has no valid destination folder");
                return;
            }

            fs::path content_root = m_tree_root.empty() ? resolve_content_browser_root() : fs::path(m_tree_root);
            fs::path engine_root = m_engine_content_root.empty() ? resolve_engine_content_root() : fs::path(m_engine_content_root);
            if (!engine_root.empty() &&
                path_is_inside_or_equal(destination_dir.lexically_normal(), engine_root.lexically_normal()))
            {
                content_root = engine_root;
            }
            if (content_root.empty())
                content_root = destination_dir;
            content_root = content_root.lexically_normal();

            AssetDatabase asset_database(content_root);
            if (!asset_database.Load())
            {
                CB_ERROR("Failed to load asset registry: {}", asset_database.RegistryPath().string().c_str());
                return;
            }

            const fs::path destination_path = make_unique_child_path(
                destination_dir / (source_path.stem().string() + ".meshasset"));

            AssetImportRequest import_request;
            import_request.sourcePath = source_path;
            import_request.destinationPath = destination_path;
            import_request.contentRoot = content_root;

            const std::string stored_asset_path = asset_database.MakeStoredPath(destination_path);
            if (const AssetRegistryRecord* existing = asset_database.Registry().FindByAssetPath(stored_asset_path))
                import_request.existingGuid = existing->guid;

            MeshImporter importer;
            AssetImportResult import_result;
            if (!importer.Import(import_request, import_result))
            {
                CB_ERROR("Failed to import mesh asset: {} -> {} ({})",
                         source_path.string().c_str(),
                         destination_path.string().c_str(),
                         import_result.error.c_str());
                return;
            }

            asset_database.Registry().Upsert(import_result.registryRecord);
            if (!asset_database.Save())
            {
                CB_ERROR("Failed to save asset registry: {}", asset_database.RegistryPath().string().c_str());
                return;
            }

            m_current_folder = destination_dir.string();
            m_tree_pending_expand = m_current_folder;
            m_selected_asset = destination_path.string();
            m_selected_node_id = 0;
            m_selected_component_index = -1;
            m_show_content_browser = true;
            m_show_details_panel = true;

            CB_INFO("Imported mesh asset: {}", destination_path.string().c_str());
            CB_INFO("Updated asset registry: {}", asset_database.RegistryPath().string().c_str());
#else
            CB_WARN("Mesh import is only implemented on Windows for now");
#endif
        }

        void Editor::handle_open_scene()
        {
            if (m_selected_asset.empty())
                return;
            Core::Application* app = m_pApp ? m_pApp : Core::Application::getApp();
            Samples::SampleApp* sample_app = app ? app->get_sample_app() : nullptr;
            RefCntAutoPtr<World> world = sample_app ? sample_app->get_world() : RefCntAutoPtr<World>{};
            if (!world)
                return;
            if (!SceneSerializer::load(*world, m_selected_asset.c_str()))
                return;
            // MeshComponents listed in the file need the sample to pump pending
            // loads; queue each one now so the sample will process them on its
            // next tick.
            world->for_each_component_of<Component::MeshComponent>(
                [&world](SceneNode& n, Component::MeshComponent& mc, uint32_t idx)
                {
                    if (!mc.model_resource.empty() && !mc.gpu_ready)
                        world->enqueue_pending_load(n.id, idx, mc.model_resource);
                });
            m_selected_node_id = 0;
            m_selected_component_index = -1;
        }

        void Editor::handle_save_scene()
        {
            Core::Application* app = m_pApp ? m_pApp : Core::Application::getApp();
            Samples::SampleApp* sample_app = app ? app->get_sample_app() : nullptr;
            RefCntAutoPtr<World> world = sample_app ? sample_app->get_world() : RefCntAutoPtr<World>{};
            if (!world || world->source_path().empty())
                return;
            SceneSerializer::save(*world, world->source_path().c_str());
        }

        void Editor::draw_save_as_popup()
        {
            if (m_show_save_as_popup)
            {
                ImGui::OpenPopup("Save Scene As");
                m_show_save_as_popup = false;
            }

            ImGui::SetNextWindowSize(ImVec2(480, 0), ImGuiCond_Appearing);
            if (ImGui::BeginPopupModal("Save Scene As", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
            {
                ImGui::TextUnformatted("Project-relative or absolute path:");
                ImGui::InputText("##save_as_path", m_save_as_buffer, sizeof(m_save_as_buffer));

                if (ImGui::Button("Save", ImVec2(120, 0)))
                {
                    Core::Application* app = m_pApp ? m_pApp : Core::Application::getApp();
                    Samples::SampleApp* sample_app = app ? app->get_sample_app() : nullptr;
                    RefCntAutoPtr<World> world = sample_app ? sample_app->get_world() : RefCntAutoPtr<World>{};
                    if (world && m_save_as_buffer[0] != '\0')
                        SceneSerializer::save(*world, m_save_as_buffer);
                    ImGui::CloseCurrentPopup();
                }
                ImGui::SameLine();
                if (ImGui::Button("Cancel", ImVec2(120, 0)))
                    ImGui::CloseCurrentPopup();
                ImGui::EndPopup();
            }
        }

        bool Editor::try_accept_asset_drop(const char* utf8_path, const float3& drop_position)
        {
            if (!utf8_path || !*utf8_path)
                return false;

            Core::Application* app = m_pApp ? m_pApp : Core::Application::getApp();
            Samples::SampleApp* sample_app = app ? app->get_sample_app() : nullptr;
            RefCntAutoPtr<World> world = sample_app ? sample_app->get_world() : RefCntAutoPtr<World>{};
            if (!world)
                return false;

            std::string ext = std::filesystem::path(utf8_path).extension().string();
            std::transform(ext.begin(), ext.end(), ext.begin(),
                [](unsigned char c){ return static_cast<char>(std::tolower(c)); });

            const ResourceTypeInfo* info = ResourceTypeRegistry::get().find(ext);
            if (!info)
            {
                CB_WARN("Dropped unknown file type: %s", utf8_path);
                return false;
            }

            if (info->category != ResourceCategory::Model)
            {
                CB_INFO("Drag-drop ignored (%s): only Model assets are accepted for now",
                    info->display_name.c_str());
                return false;
            }
            if (ext == ".meshasset")
            {
                CB_INFO("Drag-drop ignored (%s): .meshasset runtime loading is not implemented yet",
                    info->display_name.c_str());
                return false;
            }

            // Convert to project-relative path for storage in the node.
            std::filesystem::path p(utf8_path);
            std::error_code ec;
            std::filesystem::path rel = std::filesystem::relative(
                p, std::filesystem::path(Core::FileHelper::get_project_root()), ec);
            eastl::string rel_str;
            if (!ec && !rel.empty() && rel.native().front() != '.')
                rel_str = rel.generic_string().c_str();
            else
                rel_str = utf8_path;

            SceneNode node;
            node.name = p.stem().string().c_str();
            auto mc = Scope<Component::MeshComponent>(new Component::MeshComponent());
            mc->model_resource = rel_str;
            mc->position = drop_position;
            node.components.push_back(Scope<Component::Primitive>(mc.release()));
            uint32_t new_id = world->add_node(std::move(node));
            world->enqueue_pending_load(new_id, 0, rel_str);
            m_selected_node_id = new_id;
            m_selected_component_index = 0;
            CB_INFO("Added node %u from %s", new_id, rel_str.c_str());
            return true;
        }

        void Editor::finalize()
        {
            for (auto& icon : m_content_browser_icons)
            {
                icon.view = nullptr;
                icon.texture.reset();
            }
            m_content_browser_icons_loaded = false;

            // Cleanup
            ImGui_ImplDX12_Shutdown();
            //ImGui_ImplWin32_Shutdown();
            ImGui::DestroyContext();
        }

        void Editor::end_frame()
        {
            m_imguiRenderer->end_frame();
            ImGui::EndFrame();
        }

        void Editor::render(RenderObject::IDeviceContext* device_context, RenderObject::IRenderDevice* device)
        {
            ImGui::Render();
            m_imguiRenderer->render_draw_data(device_context, ImGui::GetDrawData());
        }

        void Editor::invalidate_device_objects()
        {
            m_imguiRenderer->invalidate_device_objects();
        }

        void Editor::create_device_objects()
        {
            m_imguiRenderer->create_device_objects();
        }

        void Editor::update_fonts_texture()
        {
            m_imguiRenderer->create_fonts_texture();
        }
    }
}
