#pragma once
#include "cyber_game.config.h"
#include "application/application.h"
#include "gameruntime/world.h"

namespace Cyber
{
    namespace Samples
    {
        class CYBER_GAME_API SampleApp
        {
        public:
            SampleApp();
            ~SampleApp();

            virtual void initialize();
            virtual void run();
            virtual void update(float deltaTime);
            virtual void present();
            virtual void draw_ui(ImGuiContext* in_imgui_context) {}
            uint32_t get_back_buffer_index() const { return m_backBufferIndex; }
            RefCntAutoPtr<World> get_world() const { return m_world; }
            
            static SampleApp* create_sample_app();
        protected:
            uint32_t m_backBufferIndex = 0;
            Core::Application* m_pApp = nullptr;
            
            RefCntAutoPtr<World> m_world;
        };

    }
}