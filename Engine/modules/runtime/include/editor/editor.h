#pragma once
#include "graphics/backend/d3d12/graphics_types_d3d12.h"
#include "graphics/interface/swap_chain.hpp"
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

        enum EDITOR_COLOR_CONVERSION_MODE : uint8_t
        {
            /// Automatically convert colors between sRGB and linear space, depending on the active shader stage.
            EDITOR_COLOR_CONVERSION_MODE_AUTO = 0,
            /// Always use sRGB conversion.
            EDITOR_COLOR_CONVERSION_SRGB_TO_LINEAR,
            /// Do not convert colors
            EDITOR_COLOR_CONVERSION_NONE,
        };

        struct EditorCreateInfo
        {
            static constexpr uint32_t DefaultInitialVBSize = 1024;
            static constexpr uint32_t DefaultInitialIBSize = 1024;

            class RenderObject::IRenderDevice* pDevice = nullptr;
            HWND Hwnd = nullptr;

            TEXTURE_FORMAT BackBufferFmt = {};
            TEXTURE_FORMAT DepthBufferFmt = {};

            EDITOR_COLOR_CONVERSION_MODE ColorConversionMode = EDITOR_COLOR_CONVERSION_MODE_AUTO;

            uint32_t InitialVBSize = DefaultInitialVBSize;
            uint32_t InitialIBSize = DefaultInitialIBSize;

            EditorCreateInfo() noexcept {}
            EditorCreateInfo(RenderObject::IRenderDevice* pDevice, HWND hwnd, TEXTURE_FORMAT backBufferFmt, TEXTURE_FORMAT depthBufferFmt) noexcept
                : pDevice(pDevice)
                , Hwnd(hwnd)
                , BackBufferFmt(backBufferFmt)
                , DepthBufferFmt(depthBufferFmt)
            {
            }

            EditorCreateInfo(RenderObject::IRenderDevice* pDevice, HWND hwnd, RenderObject::SwapChainDesc swapChainDesc) noexcept
                : pDevice(pDevice)
                , Hwnd(hwnd)
                , BackBufferFmt(swapChainDesc.m_format)
                , DepthBufferFmt(swapChainDesc.m_depthStencilFormat)
            {
            }
        };

        class CYBER_RUNTIME_API Editor
        {
        public:
            Editor(const EditorCreateInfo& createInfo);
            ~Editor();
            
            // Non-copyable
            Editor(const Editor&) = delete;
            Editor(Editor&&) = delete;
            Editor& operator=(const Editor&) = delete;
            Editor& operator=(Editor&&) = delete;
            // Non-movable
            
            virtual void initialize(RenderObject::IRenderDevice* device, HWND hwnd);
            void run();
            void update(RenderObject::ICommandBuffer* encoder, float deltaTime);
            void finalize();

            struct RenderObject::DescriptorHeap_D3D12* g_imguiSrvDescHeap = nullptr;
            /**/
            virtual void new_frame(uint32_t renderSurfaceWidth, uint32_t renderSurfaceHeight);
            void end_frame();
            void render(RenderObject::IRenderDevice* device);
            void invalidate_device_objects();
            void create_device_objects();
            void create_fonts_texture();
        protected:
            class ImGuiRenderer* m_imguiRenderer;
        };
    }
}