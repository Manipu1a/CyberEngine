#include "editor/editor.h"
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_dx12.h"
#include "platform/memory.h"
#include "platform/configure.h"
#include "graphics/backend/d3d12/render_device_d3d12.h"
#include "graphics/backend/d3d12/command_buffer_d3d12.h"
#include "core/debug.h"
#include "editor/imgui_renderer.h"
#include "renderer/renderer.h"

namespace Cyber
{
    namespace Editor
    {
        Editor::Editor(const EditorCreateInfo& createInfo)
        {
            m_imguiRenderer = cyber_new<ImGuiRenderer>(createInfo);

            //initialize(createInfo.pDevice, createInfo.Hwnd);
        }

        Editor::~Editor()
        {

        }

        void Editor::initialize(RenderObject::IRenderDevice* device, HWND hwnd)
        {
            RenderObject::RenderDevice_D3D12_Impl* device_d3d12 = static_cast<RenderObject::RenderDevice_D3D12_Impl*>(device);
            D3D12_DESCRIPTOR_HEAP_DESC desc = {};
            desc.NumDescriptors = 1;
            desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
            desc.NodeMask = 0;

            // Setup Dear ImGui context
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO();
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
            io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
            io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

            // Setup Dear ImGui style
            ImGui::StyleColorsDark();
            
            // Setup Platform/Renderer backends
            m_imguiRenderer->initialize();
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
            if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
            {
                ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
                ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
            }
            
            if (ImGui::BeginMenuBar())
            {
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
                    ImGui::Image((ImTextureID)color_buffer, ImVec2(viewport_size.x, viewport_size.y), ImVec2(0, 1), ImVec2(1, 0), tint_col, border_col);
                }
                ImGui::EndChild();
            }
            ImGui::End();

            /*if(ImGui::Begin("Window"))
            {
                ImGui::End();
            }*/
            ImGui::ShowDemoWindow(&show_demo_window);
        }
        
        void Editor::finalize()
        {
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