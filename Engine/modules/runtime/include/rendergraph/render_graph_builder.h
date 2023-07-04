#pragma once
#include "render_graph_resource.h"

namespace Cyber
{
    namespace render_graph
    {
        class RenderGraphBuilder
        {
        public:
            RenderGraphBuilder();
            ~RenderGraphBuilder();

        public:
            RGTextureRef CreateTexture(RGTextureCreateDesc desc, const char8_t* name);

            RGBufferRef CreateBuffer(RGBufferCreateDesc desc, const char8_t* name);

            RGTextureViewRef CreateTextureView(RGTextureViewCreateDesc desc);

            void AddRenderPass();

            void AddComputePass();
            
        protected:
            eastl::vector<class ResourceNode*> resources;
            eastl::vector<class PassNode*> passes;
            eastl::vector<class ResourceNode*> culled_resources;
            eastl::vector<class PassNode*> culled_passes;
        };
    }

}
