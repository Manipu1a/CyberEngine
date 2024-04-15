#pragma once
#include "gameruntime/GameApplication.h"

#include "cyber_samples.config.h"

namespace Cyber
{
    namespace Samples
    {
        class CYBER_SAMPLES_API SampleApp : public Cyber::GameApplication
        {
        public:
            SampleApp();
            ~SampleApp();

            virtual void initialize(Cyber::WindowDesc& desc) override;
            virtual void run() override;
            virtual void update(float deltaTime) override;
            
        protected:


            RenderObject::CEDeviceContext* immediate_context = nullptr;
        };

    }
}