#pragma once
#include "cyber_game.config.h"
#include "application/application.h"

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

            virtual RenderObject::IFrameBuffer* get_frame_buffer() = 0;

            uint32_t get_back_buffer_index() const { return m_backBufferIndex; }
            static SampleApp* create_sample_app();
        protected:
            uint32_t m_backBufferIndex = 0;
            Core::Application* m_pApp = nullptr;
        };

    }
}