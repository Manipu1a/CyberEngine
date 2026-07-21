#include "rendergraph/render_graph_resource.h"
#include "platform/memory.h"
#include "rendergraph/render_graph_builder.h"
#include "rendergraph/render_graph.h"

namespace Cyber
{
    namespace render_graph
    {
        RenderObject::IBuffer* RGBuffer::GetBuffer()
        {
            if(buffer)
            {
                return buffer;
            }

            //buffer = rhi_create_buffer(device, create_desc);
            if(!buffer)
            {
                cyber_assert(false, "Failed to create buffer");
                return nullptr;
            }
            return buffer;
        }

        RenderObject::ITexture* RGTexture::GetTexture()
        {
            if(texture)
            {
                return texture;
            }

            //texture = rhi_create_texture(device, create_desc);
            if(!texture)
            {
                cyber_assert(false, "Failed to create texture");
                return nullptr;
            }
            return texture;
        }


        /////////////////////////////////////////////////
        RGPass::RGPass(ERGPassType type)
            : pass_type(type)
        {
        }

        void RGPass::setup(RenderGraphBuilder&)
        {
        }

        void RGPass::execute(RenderGraph&, RenderPassContext&)
        {
        }

        void RGPass::register_resource_access(RGRenderResource* resource, ERGResourceAccess access) CYBER_NOEXCEPT
        {
            if (!resource || !pass_node)
            {
                cyber_assert(false, "RenderGraph pass and resource must be registered before declaring access");
                return;
            }

            for (auto& existing : pass_node->resource_accesses)
            {
                if (existing.resource == resource)
                {
                    const uint8_t merged = static_cast<uint8_t>(existing.access) |
                        static_cast<uint8_t>(access);
                    existing.access = static_cast<ERGResourceAccess>(merged);
                    return;
                }
            }

            pass_node->resource_accesses.push_back({ resource, access });
        }

        RGPass& RGPass::read(RGTextureRef texture) CYBER_NOEXCEPT
        {
            register_resource_access(texture, ERGResourceAccess::Read);
            return *this;
        }

        RGPass& RGPass::read(RGBufferRef buffer) CYBER_NOEXCEPT
        {
            register_resource_access(buffer, ERGResourceAccess::Read);
            return *this;
        }

        RGPass& RGPass::write(RGTextureRef texture) CYBER_NOEXCEPT
        {
            register_resource_access(texture, ERGResourceAccess::Write);
            return *this;
        }

        RGPass& RGPass::write(RGBufferRef buffer) CYBER_NOEXCEPT
        {
            register_resource_access(buffer, ERGResourceAccess::Write);
            return *this;
        }

        RGPass& RGPass::read_write(RGTextureRef texture) CYBER_NOEXCEPT
        {
            register_resource_access(texture, ERGResourceAccess::ReadWrite);
            return *this;
        }

        RGPass& RGPass::read_write(RGBufferRef buffer) CYBER_NOEXCEPT
        {
            register_resource_access(buffer, ERGResourceAccess::ReadWrite);
            return *this;
        }

        RGRenderPass::RGRenderPass()
            : RGPass(ERGPassType::RG_RENDER_PASS)
            , pipeline(nullptr)
            , dload_action(LOAD_ACTION_CLEAR)
            , dstore_action(STORE_ACTION_STORE)
            , sload_action(LOAD_ACTION_CLEAR)
            , sstore_action(STORE_ACTION_STORE)
        {
            mrt_load_actions.resize(MAX_MRT_COUNT);
            mrt_store_actions.resize(MAX_MRT_COUNT);
            mrt_clear_values.resize(MAX_MRT_COUNT);
        }

        RGRenderPass::~RGRenderPass()
        {

        }

        void RGRenderPass::execute(RenderGraph& graph, RenderPassContext& context)
        {
            if (pass_function)
                pass_function(graph, context);
        }

        RGRenderPass& RGRenderPass::add_render_target(uint32_t mrt_index, RGTextureRef texture,
             LOAD_ACTION load_action, 
             GRAPHICS_CLEAR_VALUE clear_color, 
             STORE_ACTION store_action) CYBER_NOEXCEPT
        {
            if (!texture || mrt_index >= MAX_MRT_COUNT)
            {
                cyber_assert(false, "Invalid render target binding");
                return *this;
            }

            render_targets[mrt_index] = texture;

            if(num_render_target < mrt_index + 1)
            {
                num_render_target = mrt_index + 1;
            }

            mrt_load_actions[mrt_index] = load_action;
            mrt_store_actions[mrt_index] = store_action;
            mrt_clear_values[mrt_index] = clear_color;
            write(texture);

            return *this;
        }

        RGRenderPass& RGRenderPass::set_depthstencil(RGDepthStencilRef depthstencil, 
            LOAD_ACTION depth_load_action, 
            STORE_ACTION depth_store_action,
            LOAD_ACTION stencil_load_action, 
            STORE_ACTION stencil_store_action) CYBER_NOEXCEPT
        {
            depth_stencil = depthstencil;
            this->dload_action = depth_load_action;
            this->dstore_action = depth_store_action;
            this->sload_action = stencil_load_action;
            this->sstore_action = stencil_store_action;

            register_resource_access(depthstencil, ERGResourceAccess::Write);

            return *this;
        }

        RGRenderPass& RGRenderPass::set_depthstencil(RGTextureRef depthstencil,
            LOAD_ACTION depth_load_action,
            STORE_ACTION depth_store_action,
            LOAD_ACTION stencil_load_action,
            STORE_ACTION stencil_store_action) CYBER_NOEXCEPT
        {
            this->depth_stencil = depthstencil;
            this->dload_action = depth_load_action;
            this->dstore_action = depth_store_action;
            this->sload_action = stencil_load_action;
            this->sstore_action = stencil_store_action;

            write(depthstencil);

            return *this;
        }

        RGRenderPass& RGRenderPass::set_pipeline(RenderObject::IRenderPipeline* pipeline) CYBER_NOEXCEPT
        {
            this->pipeline = pipeline;
            return *this;
        }

        RGRenderPass& RGRenderPass::add_input(const char8_t* name, RGTextureRef texture) CYBER_NOEXCEPT
        {
            if (name)
                input_textures[name] = texture;
            read(texture);

            return *this;
        }

        RGRenderPass& RGRenderPass::set_depthstencil_read_only(RGTextureRef depthstencil,
            LOAD_ACTION depth_load_action,
            STORE_ACTION depth_store_action,
            LOAD_ACTION stencil_load_action,
            STORE_ACTION stencil_store_action) CYBER_NOEXCEPT
        {
            this->depth_stencil = depthstencil;
            this->dload_action = depth_load_action;
            this->dstore_action = depth_store_action;
            this->sload_action = stencil_load_action;
            this->sstore_action = stencil_store_action;

            read(depthstencil);

            return *this;
        }

        RGRenderPass& RGRenderPass::add_input(const char8_t* name, RGBufferRef buffer) CYBER_NOEXCEPT
        {
            if (name)
                input_buffers[name] = buffer;
            read(buffer);
            return *this;
        }

        RGComputePass::RGComputePass()
            : RGPass(ERGPassType::RG_COMPUTE_PASS)
        {
        }

        RGPresentPass::RGPresentPass()
            : RGPass(ERGPassType::RG_PRESENT_PASS)
        {

        }
        
        RGPresentPass::~RGPresentPass()
        {

        }
    }
}
