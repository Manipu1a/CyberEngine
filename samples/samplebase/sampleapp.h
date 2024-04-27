#pragma once
#include "cyber_samples.config.h"
#include "core/window.h"
#include "core/application.h"

namespace Cyber
{
    namespace Samples
    {
        class CYBER_SAMPLES_API SampleApp
        {
        public:
            SampleApp();
            ~SampleApp();

            virtual void initialize(Cyber::WindowDesc& desc);
            virtual void run();
            virtual void update(float deltaTime);
            
        protected:
            Core::Application* m_pApp;
        };

    }
}