#include "rendergraph/render_graph_builder.h"
#include "platform/memory.h"

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
        
        void RenderGraphBuilder::backend_api(ERHIBackend backend) CYBER_NOEXCEPT
        {

        }

        void RenderGraphBuilder::with_device(RHIDevice* device) CYBER_NOEXCEPT
        {
            this->device = device;
        }

        RGTextureRef RenderGraphBuilder::CreateRGTexture(RGTextureCreateDesc desc, const char8_t* name)
        {
            RGTexture* texture = cyber_new<RGTexture>();
            texture->create_desc = desc;
            texture->resource_name = name;
            
            if(resource_map.find(name) != resource_map.end())
            {
                cyber_assert(false, "Resource name already exists")
                return nullptr;
            }

            resource_map[name] = texture;
            return texture;
        }

        RGTextureRef RenderGraphBuilder::GetRGTexture(const char8_t* name)
        {
            if(resource_map.find(name) == resource_map.end())
            {
                cyber_assert(false, "Resource name not found")
                return nullptr;
            }

            if(resource_map[name]->resource_type != ERGResourceType::Texture)
            {
                cyber_assert(false, "Resource type is not texture")
                return nullptr;
            }

            return (RGTexture*)resource_map[name];   
        }

        RGBufferRef RenderGraphBuilder::CreateRGBuffer(RGBufferCreateDesc desc, const char8_t* name)
        {
            RGBuffer* buffer = cyber_new<RGBuffer>();
            buffer->create_desc = desc;
            buffer->resource_name = name;
            buffer->device = device;
            if(resource_map.find(name) != resource_map.end())
            {
                cyber_assert(false, "Resource name already exists")
                return nullptr;
            }

            resource_map[name] = buffer;
            return buffer;
        }

        RGBufferRef RenderGraphBuilder::GetRGBuffer(const char8_t* name)
        {
            if(resource_map.find(name) == resource_map.end())
            {
                cyber_assert(false, "Resource name not found")
                return nullptr;
            }

            if(resource_map[name]->resource_type != ERGResourceType::Buffer)
            {
                cyber_assert(false, "Resource type is not buffer")
                return nullptr;
            }

            return (RGBuffer*)resource_map[name];
        }

        RGTextureViewRef RenderGraphBuilder::CreateTextureView(RGTextureViewCreateDesc desc)
        {
            return nullptr;
        }

        void RenderGraphBuilder::AddRenderPass(RGRenderPass* pass)
        {
            passes.push_back(pass);
        }
    }
}