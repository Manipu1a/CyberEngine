#pragma once
#include "cyber_render_graph_config.h"
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
        

        ///////////////////////////////////////////////////////////////
        using render_pass_function = void(*)(eastl::vector<RGRenderResource*>, eastl::vector<RGRenderResource*>);

        class RGRenderPass
        {
        public:
            RGRenderPass() = default;
            virtual ~RGRenderPass() = default;

            void execute();
        public:
            eastl::vector<RGRenderResource*> input_resources;
            eastl::vector<RGRenderResource*> output_resources;
            render_pass_function pass_function;
        };
    }

    
}
