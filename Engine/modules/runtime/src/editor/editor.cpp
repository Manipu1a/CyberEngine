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
#include "renderer/renderer.h"
#include <filesystem>
#include <string>
#include <string_view>
#include <cstring>
#include <cctype>
#include <algorithm>

namespace Cyber
{
    namespace Editor
    {
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

            // Populate the content-browser resource-type whitelist and
            // root the directory tree at the current working directory.
            ResourceTypeRegistry::seed_defaults();
            {
                std::error_code ec;
                auto cwd = std::filesystem::current_path(ec);
                if (!ec)
                {
                    m_tree_root     = cwd.string();
                    m_current_folder = m_tree_root;
                }
            }

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

                    ImGui::DockBuilderDockWindow("Viewport", dock_main);
                    ImGui::DockBuilderDockWindow("Details", dock_right);
                    ImGui::DockBuilderDockWindow("Content Browser", dock_bottom);
                    ImGui::DockBuilderDockWindow("Log", dock_bottom_right);

                    ImGui::DockBuilderFinish(dockspace_id);
                    m_dock_layout_built = true;
                }

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

                if (ImGui::BeginMenu("View"))
                {
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

            // Make sure the tree root + current folder are valid.
            if (m_tree_root.empty() || !fs::exists(m_tree_root, ec) || !fs::is_directory(m_tree_root, ec))
            {
                auto cwd = fs::current_path(ec);
                if (!ec)
                    m_tree_root = cwd.string();
            }
            if (m_current_folder.empty() || !fs::exists(m_current_folder, ec) || !fs::is_directory(m_current_folder, ec))
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
                        if (is_dir && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                        {
                            m_current_folder = full;
                            m_tree_pending_expand = full;
                        }
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
                            default: break;
                        }
                    }

                    ImDrawList* dl = ImGui::GetWindowDrawList();
                    ImVec2 bar_min = ImVec2(cursor_before.x, cursor_before.y + thumb_size - 6.0f);
                    ImVec2 bar_max = ImVec2(cursor_before.x + thumb_size, cursor_before.y + thumb_size);
                    dl->AddRectFilled(bar_min, bar_max, bar_color);

                    // Category glyph centered on the tile.
                    ImVec2 text_size = ImGui::CalcTextSize(glyph);
                    ImVec2 text_pos = ImVec2(
                        cursor_before.x + (thumb_size - text_size.x) * 0.5f,
                        cursor_before.y + (thumb_size - text_size.y) * 0.5f - 4.0f);
                    dl->AddText(text_pos, IM_COL32(230, 230, 230, 255), glyph);

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

            if (m_selected_asset.empty())
            {
                ImGui::TextDisabled("No asset selected");
                ImGui::Separator();
                ImGui::TextWrapped("Select an item in the Content Browser to see its details here.");
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
            }

            ImGui::Separator();
            ImGui::TextDisabled("Properties");
            ImGui::BulletText("Inspector stub - extend per asset type");

            ImGui::End();
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