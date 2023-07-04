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

        class RGRenderResource
        {
        public:
            const char8_t* ResourceName;

        };

        class RGBuffer : public RGRenderResource
        {

        };

        class RGTexture : public RGRenderResource
        {
            
        };
    }

    
}
