#include "application/application.h"
#include "application/platform/windows/windows_application.h"
#include "platform/memory.h"
#include "core/Timestep.h"
#include "core/window.h"
#include "gameruntime/sampleapp.h"
#include "editor/editor_impl_win32.h"
#include "inputsystem/core/input_manager.h"

namespace Cyber
{
    namespace Core
    {
        Application* Application::sInstance = nullptr;

        Application::Application(const WindowDesc& desc) : m_pNewInputManager(nullptr), m_pRenderer(nullptr), m_pSampleApp(nullptr), m_pEditor(nullptr)
        {
            if(sInstance == nullptr)
                sInstance = this;
        }

        Application::~Application()
        {
            
        }

        Application* Application::getApp()
        {
            if(sInstance == nullptr)
            {
                cyber_assert(false, "Application is not created!");
            }
            return sInstance;
        }

        void Application::initialize()
        {
            CB_CORE_INFO("Application :: initialize()");

        }

        void Application::on_window_create(HWND hwnd, uint32_t width, uint32_t height)
        {
            CB_CORE_INFO("Application :: on_window_create() - width: {0}, height: {1}", width, height);

            m_pRenderer = cyber_new<Renderer::Renderer>();
            m_pRenderer->create_gfx_objects();
            //m_pInputSystem = cyber_new<InputSystem>();
            //m_pInputSystem->initInputSystem();
            
            // Initialize new input system
            m_pNewInputManager = Input::InputManager::get_instance();
            m_pNewInputManager->initialize(hwnd);
            m_pNewInputManager->set_window_size(width, height);
            Editor::EditorCreateInfo createInfo = {};
            createInfo.application = this;
            createInfo.pDevice = m_pRenderer->get_render_device();
            createInfo.swap_chain = m_pRenderer->get_swap_chain();
            createInfo.BackBufferFmt = TEX_FORMAT_RGB32_FLOAT;
            createInfo.DepthBufferFmt = TEX_FORMAT_D32_FLOAT;
            createInfo.Hwnd = hwnd;
            m_pEditor = Editor::Editor_Impl_Win32::create(createInfo);
            m_pEditor->initialize(createInfo.pDevice, createInfo.Hwnd);
            
            // Only initialize sample app if it exists
            if(m_pSampleApp) {
                m_pSampleApp->initialize();
            }
        }

        void Application::resize_window(uint32_t width, uint32_t height)
        {
            CB_CORE_INFO("Application :: resize_window() - width: {0}, height: {1}", width, height);
            if(m_pWindow)
            {
                m_pRenderer->get_swap_chain()->resize(width, height);
            }
            else
            {
                CB_CORE_ERROR("Application :: resize_window() - Window is not created!");
            }
        }

        void Application::run()
        {
            CB_CORE_INFO("Application :: run()");
            CB_CORE_INFO("Application :: run() - finished");
        }

        void Application::update(float deltaTime)
        {
            float time = 0.0f;
            // Update new input system
            if (m_pNewInputManager) {
                m_pNewInputManager->update(deltaTime);
            }
            m_pWindow->update(deltaTime);
            m_pEditor->new_frame( m_pWindow->get_width(), m_pWindow->get_height());

            m_pRenderer->begin_frame();

            m_pRenderer->update(deltaTime);

            if (m_pSampleApp->is_loading())
            {
                // Advance one loading stage per frame
                m_pSampleApp->tick_loading();

                // Acquire swap chain image and start command recording for the editor render pass
                auto* render_device = m_pRenderer->get_render_device();
                auto* swap_chain = m_pRenderer->get_swap_chain();
                AcquireNextDesc acquire_desc = {};
                uint32_t back_buffer_index = render_device->acquire_next_image(swap_chain, acquire_desc);
                m_pRenderer->set_back_buffer_index(back_buffer_index);

                m_pRenderer->get_device_context()->cmd_begin();

                // Draw loading overlay via ImGui
                m_pEditor->update(deltaTime);

                auto* imgui_ctx = m_pEditor->get_imgui_context();
                if (imgui_ctx)
                {
                    ImGui::SetCurrentContext(imgui_ctx);

                    ImGuiViewport* viewport = ImGui::GetMainViewport();
                    ImVec2 center = ImVec2(
                        viewport->Pos.x + viewport->Size.x * 0.5f,
                        viewport->Pos.y + viewport->Size.y * 0.5f
                    );

                    float bar_width = 400.0f;
                    float bar_height = 24.0f;
                    float window_width = bar_width + 40.0f;
                    float window_height = 100.0f;

                    ImGui::SetNextWindowPos(ImVec2(center.x - window_width * 0.5f, center.y - window_height * 0.5f));
                    ImGui::SetNextWindowSize(ImVec2(window_width, window_height));
                    ImGui::SetNextWindowBgAlpha(0.85f);

                    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar
                        | ImGuiWindowFlags_NoResize
                        | ImGuiWindowFlags_NoMove
                        | ImGuiWindowFlags_NoScrollbar
                        | ImGuiWindowFlags_NoCollapse
                        | ImGuiWindowFlags_NoDocking;

                    if (ImGui::Begin("##Loading", nullptr, flags))
                    {
                        const char* title = "Loading Scene";
                        float title_width = ImGui::CalcTextSize(title).x;
                        ImGui::SetCursorPosX((window_width - title_width) * 0.5f);
                        ImGui::Text("%s", title);

                        ImGui::Spacing();

                        ImGui::SetCursorPosX(20.0f);
                        ImGui::ProgressBar(m_pSampleApp->get_loading_progress(), ImVec2(bar_width, bar_height));

                        ImGui::Spacing();

                        char status[128];
                        snprintf(status, sizeof(status), "%s (%.0f%%)",
                            m_pSampleApp->get_loading_message(),
                            m_pSampleApp->get_loading_progress() * 100.0f);
                        float status_width = ImGui::CalcTextSize(status).x;
                        ImGui::SetCursorPosX((window_width - status_width) * 0.5f);
                        ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "%s", status);
                    }
                    ImGui::End();
                }

                m_pEditor->render(m_pRenderer->get_device_context(), m_pRenderer->get_render_device());
                m_pSampleApp->present();
            }
            else
            {
                // Finalize loading: run on_create_resources() in the normal frame path
                // so GPU commands are properly recorded in the same command list lifecycle
                if (m_pSampleApp->get_loading_stage() == Samples::SampleApp::LoadingStage::RESOURCES)
                {
                    m_pSampleApp->tick_loading();
                }

                const bool use_engine_forward_pipeline = m_pSampleApp->use_engine_forward_pipeline();
                m_pSampleApp->update(deltaTime);
                if (use_engine_forward_pipeline)
                {
                    auto world = m_pSampleApp->get_world();
                    m_pRenderer->render_world(world.get(), deltaTime);
                }

                m_pEditor->update(deltaTime);
                m_pSampleApp->draw_ui(m_pEditor->get_imgui_context());
                m_pEditor->render(m_pRenderer->get_device_context(), m_pRenderer->get_render_device());

                if (use_engine_forward_pipeline)
                    m_pRenderer->present();
                else
                    m_pSampleApp->present();
            }

            m_pRenderer->end_frame();
        }
        
        Application* Application::create_application(const WindowDesc& desc)
        {
            sInstance = cyber_new<Platform::WindowsApplication>(desc);
            return sInstance;
        }
    }
}
