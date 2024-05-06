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

        enum IMGUI_COLOR+
        struct EditorCreateInfo
        {
            static constexpr uint32_t DefaultInitialVBSize = 1024;
            static constexpr uint32_t DefaultInitialIBSize = 1024;

            class RenderObject::IRenderDevice* pDevice = nullptr;

            TEXTURE_FORMAT BackBufferFmt = {};
            TEXTURE_FORMAT DepthBufferFmt = {};

        };
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

            /**/
            void new_frame();
            void end_frame();
            void render();
            void invalidate_device_objects();
            void create_device_objects();
            void create_fonts_texture();

        protected:
            class ImGuiRenderer* m_imguiRenderer;
        };
    }
}