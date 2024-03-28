#pragma once
#include "common/cyber_graphics_config.h"
#include "interface/texture_view.h"
#include "device_object.h"
#include "texture.hpp"

namespace Cyber
{
    namespace RenderObject
    {
        struct CYBER_GRAPHICS_API SwapChainDesc
        {
            /// Present Queues
            class IQueue* m_presentQueue;
            /// Present Queues Count
            uint32_t m_presentQueueCount;
            /// Number of backbuffers in the swapchain
            uint32_t m_imageCount;
            /// Width of the swapchain
            uint32_t m_width;
            /// Height of the swapchain 
            uint32_t m_height;
            /// Format of the swapchain
            TEXTURE_FORMAT m_format;
            /// Surface
            class Surface* m_pSurface;
            /// Set whether swapchain will be presented using vsync
            bool m_enableVsync;
            /// We can toogle to using FLIP model if app desires
            bool m_useFlipSwapEffect;
        };

        struct CYBER_GRAPHICS_API ISwapChain : public IDeviceObject
        {
            virtual const SwapChainDesc& get_create_desc() const = 0;
        };

        template<typename EngineImplTraits>
        class SwapChainBase : public DeviceObjectBase<typename EngineImplTraits::SwapChainInterface, typename EngineImplTraits::RenderDeviceImplType>
        {
        public:
            using SwapChainInterface = typename EngineImplTraits::SwapChainInterface;
            using RenderDeviceImplType = typename EngineImplTraits::RenderDeviceImplType;
            using TSwapChainBase = typename DeviceObjectBase<SwapChainInterface, RenderDeviceImplType>;

            SwapChainBase(RenderDeviceImplType* device) : TSwapChainBase(device) {  };

            virtual ~SwapChainBase() = default;

            virtual const SwapChainDesc& get_create_desc() const
            {
                return m_desc;
            }

            virtual void free () override
            {
                for(uint32_t i = 0;i < m_bufferSRVCount; ++i)
                {
                    m_ppBackBufferSRVs[i]->free();
                }

                TSwapChainBase::free();
            }
        protected:
            SwapChainDesc m_desc;
            RenderObject::ITexture** m_ppBackBufferSRVs;
            RenderObject::ITextureView** m_ppBackBufferSRVViews;
            uint32_t m_bufferSRVCount;
            RenderObject::ITexture* m_pBackBufferDSV;
            RenderObject::ITextureView* m_pBackBufferDSVView;
        };
    }

}