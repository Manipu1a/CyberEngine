#pragma once
#include "render_graph_resource.h"
#include "EASTL/map.h"
#include "EASTL/vector.h"
namespace Cyber
{
    namespace render_graph
    {
        class RenderGraphBuilder
        {
        public:
            RenderGraphBuilder(RHIDevice* device);
            ~RenderGraphBuilder();

        public:
            RGTextureRef CreateRGTexture(RGTextureCreateDesc desc, const char8_t* name);
            RGTextureRef GetRGTexture(const char8_t* name);
            RHITexture* GetRHITexture(const char8_t* name);
            
            RGBufferRef CreateRGBuffer(RGBufferCreateDesc desc, const char8_t* name);
            RGBufferRef GetRGBuffer(const char8_t* name);

            RGTextureViewRef CreateTextureView(RGTextureViewCreateDesc desc);

            void AddRenderPass(RGRenderPass* pass);
            void AddComputePass();
        protected:
            RHIDevice* device;
            eastl::map<const char8_t*, class RGRenderResource*> resource_map;
            eastl::vector<class RGRenderResource*> resources;
            eastl::vector<class RGRenderPass*> passes;
            eastl::vector<class RGRenderResource*> culled_resources;
            eastl::vector<class RGRenderPass*> culled_passes;
        };
    }

}
