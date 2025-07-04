#pragma once
#include "common/cyber_graphics_config.h"
#include "interface/texture_view.h"
#include "device_object.h"
#include "texture.hpp"

namespace Cyber
{
    namespace RenderObject
    {
        class IDeviceContext;

        struct CYBER_GRAPHICS_API SwapChainDesc
        {
            /// Number of backbuffers in the swapchain
            uint32_t m_imageCount;
            /// Width of the swapchain
            uint32_t m_width;
            /// Height of the swapchain 
            uint32_t m_height;
            /// Format of the swapchain
            TEXTURE_FORMAT m_format;
            /// Format of the depth stencil buffer
            TEXTURE_FORMAT m_depthStencilFormat;
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

            virtual RenderObject::ITexture** get_back_buffers() const = 0;
            virtual RenderObject::ITexture* get_back_buffer(uint32_t index) const = 0;
            virtual void set_back_buffers(RenderObject::ITexture** srvs) = 0;
            virtual void set_back_buffer(RenderObject::ITexture* srv, uint32_t index) = 0;

            virtual RenderObject::ITexture_View** get_back_buffer_srv_views() const = 0;
            virtual RenderObject::ITexture_View* get_back_buffer_srv_view(uint32_t index) const = 0;
            virtual void set_back_buffer_srv_views(RenderObject::ITexture_View** srvViews) = 0;
            virtual void set_back_buffer_srv_view(RenderObject::ITexture_View* srvView, uint32_t index) = 0;

            virtual uint32_t get_buffer_srv_count() const = 0;
            virtual void set_buffer_srv_count(uint32_t count) = 0;

            virtual RenderObject::ITexture* get_back_buffer_depth() const = 0;
            virtual void set_back_buffer_depth(RenderObject::ITexture* dsv) = 0;
            virtual RenderObject::ITexture_View* get_back_buffer_dsv() const = 0;
            virtual void set_back_buffer_dsv(RenderObject::ITexture_View* dsv) = 0;

            virtual void resize(uint32_t width, uint32_t height) = 0;
        };

        template<typename EngineImplTraits>
        class SwapChainBase : public DeviceObjectBase<typename EngineImplTraits::SwapChainInterface, typename EngineImplTraits::RenderDeviceImplType>
        {
        public:
            using SwapChainInterface = typename EngineImplTraits::SwapChainInterface;
            using RenderDeviceImplType = typename EngineImplTraits::RenderDeviceImplType;
            using TSwapChainBase = DeviceObjectBase<SwapChainInterface, RenderDeviceImplType>;

            SwapChainBase(RenderDeviceImplType* device, SwapChainDesc _desc, RenderObject::IDeviceContext* _device_const)
             : TSwapChainBase(device) , swap_chain_desc(_desc), device_context(_device_const)
            {  };

            virtual ~SwapChainBase() = default;

            virtual const SwapChainDesc& get_create_desc() const override
            {
                return swap_chain_desc;
            }

            virtual void free () override
            {
                for(uint32_t i = 0;i < m_bufferSRVCount; ++i)
                {
                    m_ppBackBufferSRVs[i]->free();
                }

                TSwapChainBase::free();
            }

            virtual RenderObject::ITexture** get_back_buffers() const override
            {
                return m_ppBackBuffers;
            }

            virtual RenderObject::ITexture* get_back_buffer(uint32_t index) const override
            {
                return m_ppBackBuffers[index];
            }

            virtual void set_back_buffers(RenderObject::ITexture** srvs) override
            {
                m_ppBackBuffers = srvs;
            }

            virtual void set_back_buffer(RenderObject::ITexture* srv, uint32_t index) override
            {
                m_ppBackBuffers[index] = srv;
            }
            virtual RenderObject::ITexture_View** get_back_buffer_srv_views() const override
            {
                return m_ppBackBufferSRVs;
            }

            virtual RenderObject::ITexture_View* get_back_buffer_srv_view(uint32_t index) const override
            {
                return m_ppBackBufferSRVs[index];
            }

            virtual void set_back_buffer_srv_views(RenderObject::ITexture_View** srvViews) override
            {
                m_ppBackBufferSRVs = srvViews;
            }

            virtual void set_back_buffer_srv_view(RenderObject::ITexture_View* srvView, uint32_t index) override
            {
                m_ppBackBufferSRVs[index] = srvView;
            }

            virtual uint32_t get_buffer_srv_count() const override
            {
                return m_bufferSRVCount;
            }

            virtual void set_buffer_srv_count(uint32_t count) override
            {
                m_bufferSRVCount = count;
            }
            
            virtual RenderObject::ITexture* get_back_buffer_depth() const override
            {
                return m_pBackBufferDepth;
            }
            virtual void set_back_buffer_depth(RenderObject::ITexture* dsv) override
            {
                m_pBackBufferDepth = dsv;
            }
            virtual RenderObject::ITexture_View* get_back_buffer_dsv() const override
            {
                return m_pBackBufferDSV;
            }

            virtual void set_back_buffer_dsv(RenderObject::ITexture_View* dsv) override
            {
                m_pBackBufferDSV = dsv;
            }

            virtual void resize(uint32_t width, uint32_t height) override
            {
                swap_chain_desc.m_width = width;
                swap_chain_desc.m_height = height;
            }


        protected:
            RenderObject::IDeviceContext* device_context;

            SwapChainDesc swap_chain_desc;
            RenderObject::ITexture** m_ppBackBuffers;
            RenderObject::ITexture_View** m_ppBackBufferSRVs;
            uint32_t m_bufferSRVCount;
            RenderObject::ITexture* m_pBackBufferDepth;
            RenderObject::ITexture_View* m_pBackBufferDSV;
        };
    }

}