#pragma once
#include "render_graph_resource.h"
#include "EASTL/map.h"
#include "EASTL/vector.h"
#include "platform/configure.h"

namespace Cyber
{
    namespace render_graph
    {
        class CYBER_RUNTIME_API RenderGraphBuilder
        {
        public:
            RenderGraphBuilder();
            ~RenderGraphBuilder();
            RenderGraphBuilder& backend_api(ERHIBackend backend) CYBER_NOEXCEPT;
            RenderGraphBuilder& with_device(RHIDevice* device) CYBER_NOEXCEPT;

        public:
            RGTextureRef create_texture(RGTextureCreateDesc desc, const char8_t* name);
            RGTextureRef get_texture(const char8_t* name);

            RHITexture* GetRHITexture(const char8_t* name);
            
            RGBufferRef create_buffer(RGBufferCreateDesc desc, const char8_t* name);
            RGBufferRef get_buffer(const char8_t* name);

            RGTextureViewRef CreateTextureView(RGTextureViewCreateDesc desc);

            void add_render_pass(const char8_t* name, RGRenderPassCreateDesc desc, const render_pass_function& func);
            void add_compute_pass();
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
