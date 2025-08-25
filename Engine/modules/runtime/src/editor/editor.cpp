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
#include "renderer/renderer.h"

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
                    ImGui::Image((ImTextureID)color_buffer, ImVec2(viewport_size.x, viewport_size.y), ImVec2(0, 0), ImVec2(1, 1), tint_col, border_col);
                    
                    // Draw world coordinate arrow gizmo in bottom-left corner
                    ImVec2 window_pos = ImGui::GetWindowPos();
                    ImVec2 window_size = ImGui::GetWindowSize();

                    ImVec2 bottom_left = ImVec2(window_pos.x, window_pos.y + window_size.y);

                    // Set gizmo position and size (bottom-left corner)
                    float gizmo_size = 100.0f;
                    float gizmo_offset = 20.0f;
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
                    
                    // Draw origin sphere
                    draw_list->AddCircleFilled(gizmo_center, 5.0f, IM_COL32(255, 255, 255, 255));
                    draw_list->AddCircle(gizmo_center, 5.0f, IM_COL32(0, 0, 0, 255), 0, 1.5f);
                    
                    // Draw X axis (Red) - pointing right
                    ImVec2 x_end = ImVec2(gizmo_center.x + arrow_length, gizmo_center.y);
                    DrawArrow(gizmo_center, x_end, IM_COL32(255, 0, 0, 255), "X");
                    
                    // Draw Y axis (Green) - pointing up
                    ImVec2 y_end = ImVec2(gizmo_center.x, gizmo_center.y - arrow_length);
                    DrawArrow(gizmo_center, y_end, IM_COL32(0, 255, 0, 255), "Y");
                    
                    // Draw Z axis (Blue) - pointing diagonal (to simulate coming out of screen)
                    float z_diagonal = arrow_length * 0.707f; // 45 degree angle
                    ImVec2 z_end = ImVec2(gizmo_center.x - z_diagonal * 0.5f, gizmo_center.y - z_diagonal * 0.5f);
                    DrawArrow(gizmo_center, z_end, IM_COL32(0, 0, 255, 255), "Z");
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