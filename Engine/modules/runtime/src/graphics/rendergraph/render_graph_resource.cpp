#include "rendergraph/render_graph_resource.h"
#include "platform/memory.h"

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
        RGRenderPass::RGRenderPass()
        {
            pass_type = ERGPassType::RG_COMPUTE_PASS;
            mrt_load_actions.resize(MAX_MRT_COUNT);
            mrt_store_actions.resize(MAX_MRT_COUNT);
            pass_node = nullptr;
        }

        RGRenderPass::~RGRenderPass()
        {

        }

        RGRenderPass& RGRenderPass::add_render_target(uint32_t mrt_index, RGTextureRef texture,
             LOAD_ACTION load_action, 
             GRAPHICS_CLEAR_VALUE clear_color, 
             STORE_ACTION store_action) CYBER_NOEXCEPT
        {
            render_targets[mrt_index] = texture;

            if(num_render_target < mrt_index + 1)
            {
                num_render_target = mrt_index + 1;
            }

            mrt_load_actions[mrt_index] = load_action;
            mrt_store_actions[mrt_index] = store_action;

            TextureWriteEdge* edge = cyber_new<TextureWriteEdge>(texture, pass_node);
            pass_node->write_edges.push_back(edge);

            return *this;
        }

        RGRenderPass& RGRenderPass::set_depthstencil(RGDepthStencilRef depthstencil, 
            LOAD_ACTION depth_load_action, 
            STORE_ACTION depth_store_action,
            LOAD_ACTION stencil_load_action, 
            STORE_ACTION stencil_store_action) CYBER_NOEXCEPT
        {
            this->dload_action = depth_load_action;
            this->dstore_action = depth_store_action;
            this->sload_action = stencil_load_action;
            this->sstore_action = stencil_store_action;

            return *this;
        }

        RGRenderPass& RGRenderPass::set_pipeline(IRenderPipeline* pipeline) CYBER_NOEXCEPT
        {
            this->pipeline = pipeline;
            return *this;
        }

        RGRenderPass& RGRenderPass::add_input(const char8_t* name, RGTextureRef texture) CYBER_NOEXCEPT
        {
            input_textures[name] = texture;
            TextureReadEdge* edge = cyber_new<TextureReadEdge>(texture, pass_node);
            pass_node->read_edges.push_back(edge);

            return *this;
        }

        RGRenderPass& RGRenderPass::add_input(const char8_t* name, RGBufferRef buffer) CYBER_NOEXCEPT
        {
            input_buffers[name] = buffer;
            return *this;
        }

        RGPresentPass::RGPresentPass()
        {

        }
        
        RGPresentPass::~RGPresentPass()
        {

        }
    }
}