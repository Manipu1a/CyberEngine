#include "graphics/backend/d3d12/swap_chain_d3d12.h"
#include "platform/memory.h"
#include "graphics/backend/d3d12/d3d12_utils.h"
#include "graphics/backend/d3d12/device_context_d3d12.h"
#include "graphics/backend/d3d12/render_device_d3d12.h"
namespace Cyber
{
    namespace RenderObject
    {
        SwapChain_D3D12_Impl::SwapChain_D3D12_Impl(RenderObject::RenderDevice_D3D12_Impl* device, SwapChainDesc desc, RenderObject::IDeviceContext* _device_context) 
        : TSwapChainBase(device, desc, _device_context)
        {
            m_pDxSwapChain = nullptr;
            m_dxSyncInterval = 1;
            m_flags = 0;
            m_imageCount = 0;
            m_enableVsync = 0;
        }

        void SwapChain_D3D12_Impl::resize(uint32_t width, uint32_t height)
        {
            if(m_pDxSwapChain == nullptr)
            {
                CB_CORE_ERROR("SwapChain_D3D12_Impl::resize - Swap chain is not initialized!");
                return;
            }
            if(swap_chain_desc.m_width == width && swap_chain_desc.m_height == height)
            {
                // No need to resize
                return;
            }

            // Wait for the GPU to finish any work that may reference the
            // current backbuffers.
            render_device->idle_command_queue();

            // Close + Reset the current command list. This drops the runtime's
            // internal tracking of resources referenced by previously recorded
            // commands — DXGI's ResizeBuffers requires all outstanding
            // references to the backbuffers to be gone, including the ones
            // held by the command list itself.
            if(auto* dx_device_context = static_cast<DeviceContext_D3D12_Impl*>(device_context))
            {
                dx_device_context->release_stale_resource_references();
            }

            TSwapChainBase::resize(width, height);

            // Release our own backbuffer wrappers first, then explicitly
            // release the raw COM pointers from GetBuffer().
            if(m_pBackBufferDSV)
            {
                m_pBackBufferDSV.reset();
            }
            if(m_pBackBufferDepth)
            {
                m_pBackBufferDepth.reset();
            }
            if(m_ppBackBufferSRVs.size() > 0)
            {
                for(uint32_t i = 0; i < m_ppBackBufferSRVs.size(); ++i)
                {
                    m_ppBackBufferSRVs[i].reset();
                }
                m_ppBackBufferSRVs.clear();
            }
            if(m_ppBackBuffers.size() > 0)
            {
                for(uint32_t i = 0; i < m_ppBackBuffers.size(); ++i)
                {
                    m_ppBackBuffers[i].reset();
                }
                m_ppBackBuffers.clear();
            }

            // Explicitly release the COM references obtained from GetBuffer().
            // The Texture wrappers do NOT own these (owns_native_resource=false),
            // so we must release them here to satisfy ResizeBuffers.
            for(uint32_t i = 0; i < m_dxBackBufferResources.size(); ++i)
            {
                if(m_dxBackBufferResources[i])
                {
                    m_dxBackBufferResources[i]->Release();
                    m_dxBackBufferResources[i] = nullptr;
                }
            }
            m_dxBackBufferResources.clear();

            // Resize the swap chain now that all references are released.
            DXGI_SWAP_CHAIN_DESC SCDes;
            memset(&SCDes, 0, sizeof(SCDes));
            m_pDxSwapChain->GetDesc(&SCDes);
            CHECK_HRESULT(m_pDxSwapChain->ResizeBuffers(SCDes.BufferCount, width, height, SCDes.BufferDesc.Format, SCDes.Flags));

            init_buffers_and_views();
        }

        void SwapChain_D3D12_Impl::free()
        {
            for(uint32_t i = 0; i < m_dxBackBufferResources.size(); ++i)
            {
                if(m_dxBackBufferResources[i])
                {
                    m_dxBackBufferResources[i]->Release();
                    m_dxBackBufferResources[i] = nullptr;
                }
            }
            m_dxBackBufferResources.clear();
            SAFE_RELEASE(m_pDxSwapChain);
        }

        void SwapChain_D3D12_Impl::init_buffers_and_views()
        {
            auto buffer_count = swap_chain_desc.m_imageCount;
            // Get swapchain images
            m_ppBackBufferSRVs.resize(buffer_count);
            m_ppBackBuffers.resize(buffer_count);
            m_dxBackBufferResources.resize(buffer_count);

            TextureCreateDesc textureDesc = {};
            for(uint32_t i = 0; i < buffer_count; ++i)
            {
                CHECK_HRESULT(m_pDxSwapChain->GetBuffer(i, IID_PPV_ARGS(&m_dxBackBufferResources[i])));
                textureDesc.m_width = swap_chain_desc.m_width;
                textureDesc.m_height = swap_chain_desc.m_height;
                textureDesc.m_depth = 1;
                textureDesc.m_arraySize = 1;
                textureDesc.m_format = swap_chain_desc.m_format;
                textureDesc.m_mipLevels = 1;
                textureDesc.m_sampleCount = SAMPLE_COUNT_1;
                textureDesc.m_bindFlags = GRAPHICS_RESOURCE_BIND_RENDER_TARGET;
                textureDesc.m_initializeState = GRAPHICS_RESOURCE_STATE_RENDER_TARGET;
                textureDesc.m_name = u8"SwapChain Back Buffer";
                textureDesc.m_pNativeHandle = m_dxBackBufferResources[i];
                textureDesc.m_clearValue = fastclear_1111;
                RefCntAutoPtr<RenderObject::ITexture> Ts;
                render_device->create_texture(textureDesc, nullptr, &Ts);
                m_ppBackBuffers[i] = Ts;

                auto back_buffer_view = Ts->get_default_texture_view(TEXTURE_VIEW_RENDER_TARGET);
                m_ppBackBufferSRVs[i] = back_buffer_view;
            }
            //dxSwapChain->mBackBuffers = Ts;
            m_bufferSRVCount = buffer_count;

            // Create depth stencil view
            //dxSwapChain->mBackBufferDSV = (RHITexture*)cyber_malloc(sizeof(RHITexture));
            TextureCreateDesc depthStencilDesc = {};
            depthStencilDesc.m_height = swap_chain_desc.m_height;
            depthStencilDesc.m_width = swap_chain_desc.m_width;
            depthStencilDesc.m_depth = 1;
            depthStencilDesc.m_arraySize = 1;
            depthStencilDesc.m_format = TEX_FORMAT_D24_UNORM_S8_UINT;
            depthStencilDesc.m_mipLevels = 1;
            depthStencilDesc.m_sampleCount = SAMPLE_COUNT_1;
            depthStencilDesc.m_bindFlags = GRAPHICS_RESOURCE_BIND_DEPTH_STENCIL;
            depthStencilDesc.m_initializeState = GRAPHICS_RESOURCE_STATE_DEPTH_WRITE;
            depthStencilDesc.m_name = u8"Main Depth Stencil";
            depthStencilDesc.m_clearValue.depth = 1.0f;
            depthStencilDesc.m_clearValue.stencil = 0;
            render_device->create_texture(depthStencilDesc, nullptr, &m_pBackBufferDepth);

            //auto dsv = static_cast<RenderObject::Texture_D3D12_Impl*>(depth_buffer);

            TextureViewCreateDesc depthStencilViewDesc = {};
            depthStencilViewDesc.p_texture = m_pBackBufferDepth;
            depthStencilViewDesc.dimension = TEX_DIMENSION_2D;
            depthStencilViewDesc.format = TEX_FORMAT_D24_UNORM_S8_UINT;
            depthStencilViewDesc.view_type = TEXTURE_VIEW_DEPTH_STENCIL;
            depthStencilViewDesc.aspects = TVA_DEPTH;
            depthStencilViewDesc.arrayLayerCount = 1;
            m_pBackBufferDSV = render_device->create_texture_view(depthStencilViewDesc);
        }
    }
}