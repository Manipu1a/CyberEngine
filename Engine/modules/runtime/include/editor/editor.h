#pragma once
#include "graphics/backend/d3d12/graphics_types_d3d12.h"
#include "graphics/backend/d3d12/descriptor_heap_d3d12.h"
#include "cyber_runtime.config.h"

namespace Cyber
{
    namespace RenderObject
    {
        class IRenderDevice;
    }

    namespace Editor
    {
        class CYBER_RUNTIME_API Editor
        {
        public:
            Editor();
            ~Editor();

            void initialize(RenderObject::IRenderDevice* device, HWND hwnd);
            void run();
            void update(RenderObject::ICommandBuffer* encoder, float deltaTime);
            void finalize();

            struct RenderObject::DescriptorHeap_D3D12* g_imguiSrvDescHeap = nullptr;
        };
    }
}