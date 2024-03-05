#pragma once
#include "common/cyber_graphics_config.h"
#include "interface/texture_view.h"
#include "render_object.h"

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
            class RHISurface* surface;
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
        class SwapChainBase : public RenderObjectBase<typename EngineImplTraits::SwapChainInterface, typename EngineImplTraits::RenderDeviceImplType>
        {
        public:
            using RenderDeviceImplType = typename EngineImplTraits::RenderDeviceImplType;

            SwapChainBase(RenderDeviceImplType* device);

            virtual ~SwapChainBase() = default;

            virtual const SwapChainDesc& get_create_desc() const
            {
                return create_desc;
            }
        protected:
            SwapChainDesc create_desc;
            RenderObject::ITexture** mBackBufferSRVs;
            RenderObject::ITextureView** mBackBufferSRVViews;
            uint32_t mBufferSRVCount;
            RenderObject::ITexture* mBackBufferDSV;
            RenderObject::ITextureView* mBackBufferDSVView;
        };
    }

}