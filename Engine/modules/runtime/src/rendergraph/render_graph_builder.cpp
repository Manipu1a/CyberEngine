#include "rendergraph/render_graph_builder.h"
#include "platform/memory.h"
#include "rendergraph/render_graph.h"

namespace Cyber
{
    namespace render_graph
    {
        RenderGraphBuilder::RenderGraphBuilder()
        {

        }

        RenderGraphBuilder::~RenderGraphBuilder()
        {

        }
        
        RenderGraphBuilder& RenderGraphBuilder::backend_api(ERHIBackend backend) CYBER_NOEXCEPT
        {
            return *this;
        }

        RenderGraphBuilder& RenderGraphBuilder::with_device(RHIDevice* device) CYBER_NOEXCEPT
        {
            this->device = device;
            return *this;
        }

        RenderGraphBuilder& RenderGraphBuilder::with_queue(RHIQueue* queue) CYBER_NOEXCEPT
        {
            this->gfx_queue = queue;
            return *this;
        }

        RGTextureRef RenderGraphBuilder::create_texture(RGTextureCreateDesc desc, const char8_t* name)
        {
            RGTexture* texture = cyber_new<RGTexture>();
            texture->create_desc = desc;
            texture->resource_name = name;
            
            if(graph->resource_map.find(name) != graph->resource_map.end())
            {
                cyber_assert(false, "Resource name already exists")
                return nullptr;
            }

            graph->resource_map[name] = texture;
            return texture;
        }

        RGTextureRef RenderGraphBuilder::get_texture(const char8_t* name)
        {
            if(graph->resource_map.find(name) == graph->resource_map.end())
            {
                cyber_assert(false, "Resource name not found")
                return nullptr;
            }

            if(graph->resource_map[name]->resource_type != ERGResourceType::Texture)
            {
                cyber_assert(false, "Resource type is not texture")
                return nullptr;
            }

            return (RGTexture*)graph->resource_map[name];   
        }

        RGBufferRef RenderGraphBuilder::create_buffer(RGBufferCreateDesc desc, const char8_t* name)
        {
            RGBuffer* buffer = cyber_new<RGBuffer>();
            buffer->create_desc = desc;
            buffer->resource_name = name;
            buffer->device = device;
            if(graph->resource_map.find(name) != graph->resource_map.end())
            {
                cyber_assert(false, "Resource name already exists")
                return nullptr;
            }

            graph->resource_map[name] = buffer;
            return buffer;
        }

        RGBufferRef RenderGraphBuilder::get_buffer(const char8_t* name)
        {
            if(graph->resource_map.find(name) == graph->resource_map.end())
            {
                cyber_assert(false, "Resource name not found")
                return nullptr;
            }

            if(graph->resource_map[name]->resource_type != ERGResourceType::Buffer)
            {
                cyber_assert(false, "Resource type is not buffer")
                return nullptr;
            }

            return (RGBuffer*)graph->resource_map[name];
        }

        RGTextureViewRef RenderGraphBuilder::CreateTextureView(RGTextureViewCreateDesc desc)
        {
            return nullptr;
        }

        void RenderGraphBuilder::add_render_pass(const char8_t* name, const render_pass_function& pre_func, const render_pass_execute_function& execute_func)
        {
            RenderPassNode * node = cyber_new<RenderPassNode>();
            RGRenderPass* pass = cyber_new<RGRenderPass>();
            pass->pass_name = name;
            node->pass_handle = pass;
            
            pre_func(*pass);
            pass->pass_function = execute_func;
            graph->passes.push_back(pass);
        }
    }
}