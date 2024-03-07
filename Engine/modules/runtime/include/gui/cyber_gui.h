#pragma once
#include "graphics/backend/d3d12/graphics_types_d3d12.h"
#include "graphics/interface/render_device.h"
#include "cyber_runtime.config.h"


namespace Cyber
{
    namespace GUI
    {
        class CYBER_RUNTIME_API GUIApplication
        {
        public:
            GUIApplication();
            ~GUIApplication();

            void initialize(RenderObject::IRenderDevice* device, HWND hwnd);
            void run();
            void update(RHIRenderPassEncoder* encoder, float deltaTime);
            void finalize();

            struct Cyber::RHIDescriptorHeap_D3D12* g_imguiSrvDescHeap = nullptr;
        };
    }
}