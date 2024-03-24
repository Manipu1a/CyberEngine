#pragma once
#include "common/cyber_graphics_config.h"
#include "interface/texture_view.h"
#include "device_object.h"

namespace Cyber
{
    namespace RenderObject
    {
        struct CYBER_GRAPHICS_API SwapChainDesc
        {
            /// Present Queues
            class IQueue* mPresentQueue;
            /// Present Queues Count
            uint32_t mPresentQueueCount;
            /// Number of backbuffers in the swapchain
            uint32_t mImageCount;
            /// Width of the swapchain
            uint32_t mWidth;
            /// Height of the swapchain 
            uint32_t mHeight;
            /// Format of the swapchain
            TEXTURE_FORMAT mFormat;
            /// Surface
            class Surface* surface;
            /// Set whether swapchain will be presented using vsync
            bool mEnableVsync;
            /// We can toogle to using FLIP model if app desires
            bool mUseFlipSwapEffect;
        };

        struct CYBER_GRAPHICS_API ISwapChain
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