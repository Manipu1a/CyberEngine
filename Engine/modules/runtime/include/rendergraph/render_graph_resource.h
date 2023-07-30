#pragma once
#include "cyber_render_graph_config.h"
#include "EASTL/map.h"
#include "rhi/rhi.h"

namespace Cyber
{
    namespace render_graph
    {
        class RGBuffer;
        using RGBufferRef = RGBuffer*;

        class RGBufferSRV;
        using RGBufferSRVRef = RGBufferSRV*;

        class RGBufferUAV;
        using RGBufferUAVRef = RGBufferUAV*;

        class RGTexture;
        using RGTextureRef = RGTexture*;

        class RGTextureView;
        using RGTextureViewRef = RGTextureView*;

        class RGTextureSRV;
        using RGTextureSRVRef = RGTextureSRV*;

        class RGTextureUAV;
        using RGTextureUAVRef = RGTextureUAV*;

        class RGUniformBuffer;
        using RGUniformBufferRef = RGUniformBuffer*;

        class RGDepthStencil;
        using RGDepthStencilRef = RGDepthStencil*;

        class RGRenderPass;
        class RGComputePass;

        using RGTextureCreateDesc = TextureCreateDesc;
        using RGBufferCreateDesc = BufferCreateDesc;
        using RGTextureViewCreateDesc = TextureViewCreateDesc;

        enum class ERGResourceType
        {
            Buffer,
            Texture,
            TextureView,
            BufferSRV,
            BufferUAV,
            TextureSRV,
            TextureUAV,
            UniformBuffer
        };

        class RGRenderResource
        {
        public:
            const char8_t* resource_name;
            ERGResourceType resource_type;
            RHIDevice* device;
        };

        class RGBuffer : public RGRenderResource
        {
        public:
            RGBuffer() : RGRenderResource()
            {
                resource_type = ERGResourceType::Buffer;
            }
            RGBufferCreateDesc create_desc;
            RHIBuffer* buffer;

            RHIBuffer* GetBuffer();
        };

        class RGTexture : public RGRenderResource
        {
        public:
            RGTexture() : RGRenderResource()
            {
                resource_type = ERGResourceType::Texture;
            }
            RGTextureCreateDesc create_desc;
            RHITexture* texture;
            RHITexture* GetTexture();
        };
        
        class RGDepthStencil : public RGRenderResource
        {
        public:

        };

        ///////////////////////////////////////////////////////////////
        struct RGRenderPassCreateDesc
        {
            RHIRenderPipeline* pipeline;

            RGRenderPassCreateDesc& add_render_target(uint32_t index, RGTextureRef texture,
             ERHILoadAction load_action = RHI_LOAD_ACTION_CLEAR, 
             ERHIClearValue clear_color = fastclear_0000, 
             ERHIStoreAction store_action = RHI_STORE_ACTION_STORE)
            {
                render_targets[index] = texture;

                return *this;
            }

            RGRenderPassCreateDesc& set_depthstencil(RGDepthStencilRef depthstencil, 
            ERHILoadAction depth_load_action = RHI_LOAD_ACTION_CLEAR, 
            ERHIStoreAction depth_store_action = RHI_STORE_ACTION_STORE,
            ERHILoadAction stencil_load_action = RHI_LOAD_ACTION_CLEAR, 
            ERHIStoreAction stencil_store_action = RHI_STORE_ACTION_STORE)
            {
                return *this;
            }

            RGRenderPassCreateDesc& add_input(const char8_t* name, RGTextureRef texture)
            {
                input_textures[name] = texture;
                return *this;
            }

            RGRenderPassCreateDesc& add_input(const char8_t* name, RGBufferRef buffer)
            {
                input_buffers[name] = buffer;
                return *this;
            }

            eastl::map<uint32_t, RGTextureRef> render_targets;
            eastl::map<const char8_t*, RGTextureRef> input_textures;
            eastl::map<const char8_t*, RGBufferRef> input_buffers;
        };

        using render_pass_function = eastl::function<void(RGRenderPassCreateDesc&)>;

        class RGRenderPass
        {
        public:
            RGRenderPass() = default;
            virtual ~RGRenderPass() = default;

            void execute();
        public:
            const char8_t* resource_name;
            render_pass_function pass_function;
            RGRenderPassCreateDesc create_desc;
        };
    }

    
}
