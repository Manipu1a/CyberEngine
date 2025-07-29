#include "graphics/backend/d3d12/swap_chain_d3d12.h"
#include "platform/memory.h"
#include "graphics/backend/d3d12/d3d12_utils.h"
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
            //device_context->flush();
            TSwapChainBase::resize(width, height);
            // Clear old buffers
            if(m_ppBackBufferSRVs)
            {
                for(uint32_t i = 0; i < m_bufferSRVCount; ++i)
                {
                    m_ppBackBufferSRVs[i]->free();
                }
                cyber_free(m_ppBackBufferSRVs);
                m_ppBackBufferSRVs = nullptr;
            }
            if(m_ppBackBuffers)
            {
                for(uint32_t i = 0; i < m_bufferSRVCount; ++i)
                {
                    m_ppBackBuffers[i]->free();
                }
                cyber_free(m_ppBackBuffers);
                m_ppBackBuffers = nullptr;
            }
            if(m_pBackBufferDSV)
            {
                m_pBackBufferDSV->free();
                m_pBackBufferDSV = nullptr;
            }
            if(m_pBackBufferDepth)
            {
                m_pBackBufferDepth->free();
                m_pBackBufferDepth = nullptr;
            }

            //todo idle the GPU
            render_device->idle_command_queue();

            DXGI_SWAP_CHAIN_DESC SCDes;
            memset(&SCDes, 0, sizeof(SCDes));
            m_pDxSwapChain->GetDesc(&SCDes);
            CHECK_HRESULT(m_pDxSwapChain->ResizeBuffers(SCDes.BufferCount, width, height, SCDes.BufferDesc.Format, SCDes.Flags));

            init_buffers_and_views();
        }

        void SwapChain_D3D12_Impl::free()
        {
            SAFE_RELEASE(m_pDxSwapChain);
        }

        void SwapChain_D3D12_Impl::init_buffers_and_views()
        {
            auto buffer_count = swap_chain_desc.m_imageCount;
            // Get swapchain images
            m_ppBackBufferSRVs = (RenderObject::ITexture_View**)cyber_malloc(sizeof(RenderObject::ITexture_View*) * buffer_count);
            m_ppBackBuffers = (RenderObject::ITexture**)cyber_malloc(buffer_count * sizeof(RenderObject::ITexture*));

            ID3D12Resource** backbuffers = (ID3D12Resource**)alloca(buffer_count * sizeof(ID3D12Resource*));
            TextureCreateDesc textureDesc = {};
            for(uint32_t i = 0; i < buffer_count; ++i)
            {
                CHECK_HRESULT(m_pDxSwapChain->GetBuffer(i, IID_PPV_ARGS(&backbuffers[i])));
                TextureViewCreateDesc textureViewDesc = {};
                textureViewDesc.dimension = TEX_DIMENSION_2D;
                textureViewDesc.format = swap_chain_desc.m_format;
                textureViewDesc.view_type = TEXTURE_VIEW_RENDER_TARGET;
                textureViewDesc.aspects = TVA_COLOR;
                textureViewDesc.arrayLayerCount = 1;
                textureViewDesc.p_native_resource = backbuffers[i];
                textureViewDesc.mipLevelCount = 1;
                textureViewDesc.baseMipLevel = 0;
                auto back_buffer_view = render_device->create_texture_view(textureViewDesc);
                m_ppBackBufferSRVs[i] = back_buffer_view;

                textureDesc.m_width = swap_chain_desc.m_width;
                textureDesc.m_height = swap_chain_desc.m_height;
                textureDesc.m_depth = 1;
                textureDesc.m_arraySize = 1;
                textureDesc.m_format = swap_chain_desc.m_format;
                textureDesc.m_mipLevels = 1;
                textureDesc.m_sampleCount = SAMPLE_COUNT_1;
                textureDesc.m_bindFlags = GRAPHICS_RESOURCE_BIND_RENDER_TARGET;
                textureDesc.m_initializeState = GRAPHICS_RESOURCE_STATE_RENDER_TARGET;
                textureDesc.m_name = "SwapChain Back Buffer";
                textureDesc.m_pNativeHandle = backbuffers[i];
                textureDesc.m_clearValue = fastclear_1111;
                auto Ts = render_device->create_texture(textureDesc);
                m_ppBackBuffers[i] = Ts;
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
            depthStencilDesc.m_name = "Main Depth Stencil";
            depthStencilDesc.m_clearValue.depth = 1.0f;
            depthStencilDesc.m_clearValue.stencil = 0;
            auto depth_buffer = render_device->create_texture(depthStencilDesc);
            m_pBackBufferDepth = depth_buffer;

            //auto dsv = static_cast<RenderObject::Texture_D3D12_Impl*>(depth_buffer);

            TextureViewCreateDesc depthStencilViewDesc = {};
            depthStencilViewDesc.p_texture = depth_buffer;
            depthStencilViewDesc.dimension = TEX_DIMENSION_2D;
            depthStencilViewDesc.format = TEX_FORMAT_D24_UNORM_S8_UINT;
            depthStencilViewDesc.view_type = TEXTURE_VIEW_DEPTH_STENCIL;
            depthStencilViewDesc.aspects = TVA_DEPTH;
            depthStencilViewDesc.arrayLayerCount = 1;
            m_pBackBufferDSV = render_device->create_texture_view(depthStencilViewDesc);
        }
    }
}