#pragma once
#include "EASTL/map.h"
#include "base_type.h"
#include "render_graph.h"
#include "interface/texture.hpp"
#include "interface/texture_view.h"
#include "interface/buffer.h"
#include "cyber_render_graph_config.h"

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

        class RGDepthStencil;
        using RGDepthStencilRef = RGDepthStencil*;

        class RGRenderPass;
        class RGComputePass;

        using RGTextureCreateDesc = RenderObject::TextureCreateDesc;
        using RGBufferCreateDesc = RenderObject::BufferCreateDesc;
        using RGTextureViewCreateDesc = RenderObject::TextureViewCreateDesc;

        #define MAX_MRT_COUNT 8

        enum class ERGResourceType
        {
            Buffer,
            Texture,
            TextureView,
            BufferSRV,
            BufferUAV,
            TextureSRV,
            TextureUAV,
            UniformBuffer
        };

        class RGRenderResource
        {
        public:
            const char8_t* resource_name;
            ERGResourceType resource_type;
            RHIDevice* device;
        };

        class RGBuffer : public RGRenderResource
        {
        public:
            RGBuffer() : RGRenderResource()
            {
                resource_type = ERGResourceType::Buffer;
            }
            RenderObject::IBuffer* GetBuffer();

            RGBufferCreateDesc create_desc;
            RenderObject::IBuffer* buffer;
        };

        class RGTexture : public RGRenderResource
        {
        public:
            RGTexture() : RGRenderResource()
            {
                resource_type = ERGResourceType::Texture;
            }
            RenderObject::ITexture* GetTexture();

            RGTextureCreateDesc create_desc;
            RenderObject::ITexture* texture;
            struct TextureNode* texture_node;
        };
        
        class RGDepthStencil : public RGRenderResource
        {
        public:

        };
        ///////////////////////////////////////////////////////////////
        class CYBER_RUNTIME_API RGPass
        {
        public:
            RGPass() = default;
            virtual ~RGPass() = default;

            ERGPassType pass_type;
        };


        using render_pass_function = eastl::function<void(RGRenderPass&)>;
        using render_pass_execute_function = eastl::function<void(RenderGraph&, RenderPassContext&)>;

        class CYBER_RUNTIME_API RGRenderPass : public RGPass
        {
        public:
            RGRenderPass();
            virtual ~RGRenderPass();

        public:
            RGRenderPass& add_render_target(uint32_t mrt_index, RGTextureRef texture,
             ERHILoadAction load_action = RHI_LOAD_ACTION_CLEAR, 
             ERHIClearValue clear_color = fastclear_0000, 
             ERHIStoreAction store_action = RHI_STORE_ACTION_STORE) CYBER_NOEXCEPT;

            RGRenderPass& set_depthstencil(RGDepthStencilRef depthstencil, 
            ERHILoadAction depth_load_action = RHI_LOAD_ACTION_CLEAR, 
            ERHIStoreAction depth_store_action = RHI_STORE_ACTION_STORE,
            ERHILoadAction stencil_load_action = RHI_LOAD_ACTION_CLEAR, 
            ERHIStoreAction stencil_store_action = RHI_STORE_ACTION_STORE) CYBER_NOEXCEPT;

            RGRenderPass& set_pipeline(RHIRenderPipeline* pipeline) CYBER_NOEXCEPT;
            RGRenderPass& add_input(const char8_t* name, RGTextureRef texture) CYBER_NOEXCEPT;
            RGRenderPass& add_input(const char8_t* name, RGBufferRef buffer) CYBER_NOEXCEPT;

            RHIRenderPipeline* pipeline;
            eastl::map<uint32_t, RGTextureRef> render_targets;
            eastl::map<const char8_t*, RGTextureRef> input_textures;
            eastl::map<const char8_t*, RGBufferRef> input_buffers;

            ERHILoadAction dload_action;
            ERHIStoreAction dstore_action;
            ERHILoadAction sload_action;
            ERHIStoreAction sstore_action;

            eastl::vector<ERHILoadAction> mrt_load_actions;
            eastl::vector<ERHIStoreAction> mrt_store_actions;

            uint32_t num_render_target = 0;
            const char8_t* pass_name;
            render_pass_execute_function pass_function;
            struct RenderPassNode* pass_node;
        };

        class CYBER_RUNTIME_API RGPresentPass : public RGPass
        {
        public:
            RGPresentPass();
            virtual ~RGPresentPass();
        };

        //////////////////////////////////////////////////////////////////////////

        struct RenderPassNode : public PassNode
        {
            RenderPassNode() : PassNode(RG_RENDER_PASS_NODE) {}
        };

        struct ComputePassNode : public PassNode
        {
        };

        struct PresentPassNode : public PassNode
        {
        };

        struct ResourceNode : public RenderGraphNode
        {
            ResourceNode(ERGObjectType type) : RenderGraphNode(type) {}

        };

        struct BufferNode : public ResourceNode
        {
            RenderObject::IBuffer* buffer;
        };
 
        struct TextureNode : public ResourceNode
        {
            TextureNode() : ResourceNode(RG_TEXTURE_NODE)
            {
            }

            RenderObject::ITexture* texture;
        };

        struct TextureEdge : public RenderGraphEdge
        {
            TextureEdge(RGTextureRef tex, PassNode* from)
            {
                from_node = (RenderGraphNode*)from;
                to_node = tex->texture_node;
            }
        };

        // SRV
        struct TextureReadEdge : public TextureEdge
        {
            TextureReadEdge(RGTextureRef tex, PassNode* from) : TextureEdge(tex, from)
            {
                tex->texture_node->write_edges.push_back(this);
            }
        };

        // RTV
        struct TextureWriteEdge : public TextureEdge
        {
            TextureWriteEdge(RGTextureRef tex, PassNode* from) : TextureEdge(tex, from)
            {
                tex->texture_node->read_edges.push_back(this);
            }
        };
        // UAV
        struct TextureReadWriteEdge : public TextureEdge
        {
            TextureReadWriteEdge(RGTextureRef tex, PassNode* from) : TextureEdge(tex, from)
            {
                tex->texture_node->read_edges.push_back(this);
                tex->texture_node->write_edges.push_back(this);
            }
        };
    }
}
