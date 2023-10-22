#pragma once
#include "rhi/rhi.h"
#include "cyber_runtime.config.h"
#include "rhi/backend/d3d12/rhi_d3d12.h"

namespace Cyber
{
    namespace GUI
    {
        class CYBER_RUNTIME_API GUIApplication
        {
        public:
            GUIApplication();
            ~GUIApplication();

            void initialize(RHIDevice* device, HWND hwnd);
            void run();
            void update(RHIRenderPassEncoder* encoder, float deltaTime);
            void finalize();

            struct Cyber::RHIDescriptorHeap_D3D12* g_imguiSrvDescHeap = nullptr;
        };
    }
}