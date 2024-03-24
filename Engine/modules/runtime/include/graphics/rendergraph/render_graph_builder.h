#pragma once
#include "render_graph_resource.h"
#include "EASTL/map.h"
#include "EASTL/vector.h"
#include "platform/configure.h"
#include "interface/graphics_types.h"

namespace Cyber
{
    namespace RenderObject
    {
        struct IRenderDevice;
        struct IQueue;
    }
    
    namespace render_graph
    {
        class CYBER_RUNTIME_API RenderGraphBuilder
        {
        public:
            RenderGraphBuilder();
            ~RenderGraphBuilder();
            RenderGraphBuilder& backend_api(GRAPHICS_BACKEND backend) CYBER_NOEXCEPT;
            RenderGraphBuilder& with_device(class RenderObject::IRenderDevice* device) CYBER_NOEXCEPT;
            RenderGraphBuilder& with_queue(class RenderObject::IQueue* queue) CYBER_NOEXCEPT;
            
        public:
            RGTextureRef create_texture(RGTextureCreateDesc desc, const char8_t* name);
            RGTextureRef get_texture(const char8_t* name);

            RenderObject::ITexture* GetRHITexture(const char8_t* name);
            
            RGBufferRef create_buffer(RGBufferCreateDesc desc, const char8_t* name);
            RGBufferRef get_buffer(const char8_t* name);

            RGTextureViewRef CreateTextureView(RGTextureViewCreateDesc desc);

            void add_render_pass(const char8_t* name, const render_pass_function& func, const render_pass_execute_function& execute_func);
            void add_compute_pass();
        public:
            class RenderObject::IRenderDevice* device;
            class RenderObject::IQueue* gfx_queue;
            class RenderGraph* graph;
        };
    }

}
