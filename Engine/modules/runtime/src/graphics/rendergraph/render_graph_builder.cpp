#include "rendergraph/render_graph_builder.h"
#include "platform/memory.h"
#include "rendergraph/render_graph.h"
#include "interface/command_queue.h"

namespace Cyber
{
    namespace render_graph
    {
        RenderGraphBuilder::RenderGraphBuilder(RenderGraph* owner_graph)
            : graph(owner_graph)
        {
        }

        RenderGraphBuilder::~RenderGraphBuilder()
        {

        }
        
        RenderGraphBuilder& RenderGraphBuilder::backend_api(GRAPHICS_BACKEND backend) CYBER_NOEXCEPT
        {
            return *this;
        }

        RenderGraphBuilder& RenderGraphBuilder::with_device(RenderObject::IRenderDevice* device) CYBER_NOEXCEPT
        {
            this->device = device;
            return *this;
        }

        RenderGraphBuilder& RenderGraphBuilder::with_queue(RenderObject::IQueue* queue) CYBER_NOEXCEPT
        {
            this->gfx_queue = queue;
            return *this;
        }

        RGTextureRef RenderGraphBuilder::create_texture(RGTextureCreateDesc desc, const char8_t* name)
        {
            if (!graph || !name)
            {
                cyber_assert(false, "RenderGraph and resource name must be valid");
                return nullptr;
            }

            if(graph->resource_map.find(name) != graph->resource_map.end())
            {
                cyber_assert(false, "Resource name already exists")
                return nullptr;
            }

            RGTexture* texture = cyber_new<RGTexture>();
            texture->create_desc = desc;
            texture->resource_name = name;
            texture->device = device;

            TextureNode* texture_node = cyber_new<TextureNode>();
            texture_node->texture = nullptr;
            texture->texture_node = texture_node;

            graph->resource_map[name] = texture;
            graph->resources.push_back(texture_node);
            graph->invalidate();
            return texture;
        }

        RGTextureRef RenderGraphBuilder::get_texture(const char8_t* name)
        {
            if (!graph || !name)
                return nullptr;

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

        RGTextureRef RenderGraphBuilder::import_texture(RenderObject::ITexture* texture, const char8_t* name)
        {
            if (!texture)
            {
                cyber_assert(false, "Cannot import a null texture");
                return nullptr;
            }

            RGTextureRef imported_texture = create_texture(texture->get_create_desc(), name);
            update_imported_texture(imported_texture, texture);
            return imported_texture;
        }

        void RenderGraphBuilder::update_imported_texture(RGTextureRef texture, RenderObject::ITexture* imported_texture)
        {
            if (!texture || !imported_texture)
            {
                cyber_assert(false, "RenderGraph texture import must be valid");
                return;
            }

            texture->texture = imported_texture;
            texture->create_desc = imported_texture->get_create_desc();
            if (texture->texture_node)
                texture->texture_node->texture = imported_texture;
        }

        RenderObject::ITexture* RenderGraphBuilder::GetRHITexture(const char8_t* name)
        {
            RGTextureRef texture = get_texture(name);
            return texture ? texture->GetTexture() : nullptr;
        }

        RGBufferRef RenderGraphBuilder::create_buffer(RGBufferCreateDesc desc, const char8_t* name)
        {
            if (!graph || !name)
            {
                cyber_assert(false, "RenderGraph and resource name must be valid");
                return nullptr;
            }

            if(graph->resource_map.find(name) != graph->resource_map.end())
            {
                cyber_assert(false, "Resource name already exists")
                return nullptr;
            }

            RGBuffer* buffer = cyber_new<RGBuffer>();
            buffer->create_desc = desc;
            buffer->resource_name = name;
            buffer->device = device;

            BufferNode* buffer_node = cyber_new<BufferNode>();
            buffer_node->buffer = nullptr;
            buffer->buffer_node = buffer_node;
            graph->resource_map[name] = buffer;
            graph->resources.push_back(buffer_node);
            graph->invalidate();
            return buffer;
        }

        RGBufferRef RenderGraphBuilder::get_buffer(const char8_t* name)
        {
            if (!graph || !name)
                return nullptr;

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
            RGRenderPass* pass = cyber_new<RGRenderPass>();
            register_pass(name, pass, &destroy_typed_pass<RGRenderPass>);
            pre_func(*pass);
            pass->pass_function = execute_func;
        }

        void RenderGraphBuilder::add_compute_pass()
        {
            add_pass<RGComputePass>(u8"ComputePass");
        }

        void RenderGraphBuilder::register_pass(const char8_t* name, RGPass* pass, PassNode::PassDestroyFunction destroy_pass)
        {
            if (!graph || !pass || !destroy_pass)
            {
                cyber_assert(false, "Cannot register an invalid RenderGraph pass");
                return;
            }

            PassNode* node = nullptr;
            switch (pass->pass_type)
            {
            case RG_RENDER_PASS:
                node = cyber_new<RenderPassNode>();
                break;
            case RG_COMPUTE_PASS:
                node = cyber_new<ComputePassNode>();
                break;
            case RG_PRESENT_PASS:
                node = cyber_new<PresentPassNode>();
                break;
            default:
                cyber_assert(false, "Unknown RenderGraph pass type");
                return;
            }

            pass->pass_name = name;
            pass->pass_node = node;
            node->pass_handle = pass;
            node->destroy_pass = destroy_pass;
            graph->passes.push_back(node);
            graph->invalidate();
        }
    }
}
