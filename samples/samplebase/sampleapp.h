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
            ///-------------------------------------
            static const uint32_t MAX_FRAMES_IN_FLIGHT = 3;
            static const uint32_t BACK_BUFFER_COUNT = 3;
            RHIDevice* device = nullptr;
            RHIInstance* instance = nullptr;
            RHIAdapter* adapter = nullptr;
            RHIFence* present_fence = nullptr;
            RHIQueue* queue = nullptr;
            RHICommandPool* pool = nullptr;
            RHICommandBuffer* cmd = nullptr;
            RHISwapChain* swap_chain = nullptr;
            RHISurface* surface = nullptr;
            RHIFence* present_swmaphore = nullptr; 
            uint32_t backbuffer_index = 0;

            RHI* immediate_context = nullptr;
        };

    }
}