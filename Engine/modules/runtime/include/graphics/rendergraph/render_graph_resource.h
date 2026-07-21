#pragma once
#include "EASTL/map.h"
#include "base_type.h"
#include "interface/texture.hpp"
#include "interface/texture_view.h"
#include "interface/buffer.h"
#include "cyber_runtime.config.h"
#include "cyber_render_graph_config.h"

namespace Cyber
{
    namespace render_graph
    {
        class RenderGraph;
        class RenderGraphBuilder;

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
            UniformBuffer,
            DepthStencil
        };

        class RGRenderResource
        {
        public:
            virtual ~RGRenderResource() = default;

            const char8_t* resource_name = nullptr;
            ERGResourceType resource_type = ERGResourceType::Buffer;
            RenderObject::IRenderDevice* device = nullptr;
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
            RenderObject::IBuffer* buffer = nullptr;
            struct BufferNode* buffer_node = nullptr;
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
            RenderObject::ITexture* texture = nullptr;
            struct TextureNode* texture_node = nullptr;
        };
        
        class RGDepthStencil : public RGRenderResource
        {
        public:
            RGDepthStencil()
            {
                resource_type = ERGResourceType::DepthStencil;
            }
        };
        ///////////////////////////////////////////////////////////////
        class CYBER_RUNTIME_API RGPass
        {
        public:
            explicit RGPass(ERGPassType type);
            virtual ~RGPass() = default;

            virtual void setup(RenderGraphBuilder& builder);
            virtual void execute(RenderGraph& graph, RenderPassContext& context);

            RGPass& read(RGTextureRef texture) CYBER_NOEXCEPT;
            RGPass& read(RGBufferRef buffer) CYBER_NOEXCEPT;
            RGPass& write(RGTextureRef texture) CYBER_NOEXCEPT;
            RGPass& write(RGBufferRef buffer) CYBER_NOEXCEPT;
            RGPass& read_write(RGTextureRef texture) CYBER_NOEXCEPT;
            RGPass& read_write(RGBufferRef buffer) CYBER_NOEXCEPT;

            ERGPassType pass_type;
            const char8_t* pass_name = nullptr;
            struct PassNode* pass_node = nullptr;

        protected:
            void register_resource_access(RGRenderResource* resource, ERGResourceAccess access) CYBER_NOEXCEPT;
        };


        using render_pass_function = eastl::function<void(RGRenderPass&)>;
        using render_pass_execute_function = eastl::function<void(RenderGraph&, RenderPassContext&)>;

        class CYBER_RUNTIME_API RGRenderPass : public RGPass
        {
        public:
            RGRenderPass();
            virtual ~RGRenderPass();
            void execute(RenderGraph& graph, RenderPassContext& context) override;

        public:
            RGRenderPass& add_render_target(uint32_t mrt_index, RGTextureRef texture,
            LOAD_ACTION load_action = LOAD_ACTION_CLEAR, 
            GRAPHICS_CLEAR_VALUE clear_color = fastclear_0000, 
            STORE_ACTION store_action = STORE_ACTION_STORE) CYBER_NOEXCEPT;

            RGRenderPass& set_depthstencil(RGDepthStencilRef depthstencil, 
            LOAD_ACTION depth_load_action = LOAD_ACTION_CLEAR, 
            STORE_ACTION depth_store_action = STORE_ACTION_STORE,
            LOAD_ACTION stencil_load_action = LOAD_ACTION_CLEAR, 
            STORE_ACTION stencil_store_action = STORE_ACTION_STORE) CYBER_NOEXCEPT;

            RGRenderPass& set_depthstencil(RGTextureRef depthstencil,
            LOAD_ACTION depth_load_action = LOAD_ACTION_CLEAR,
            STORE_ACTION depth_store_action = STORE_ACTION_STORE,
            LOAD_ACTION stencil_load_action = LOAD_ACTION_CLEAR,
            STORE_ACTION stencil_store_action = STORE_ACTION_STORE) CYBER_NOEXCEPT;

            RGRenderPass& set_depthstencil_read_only(RGTextureRef depthstencil,
            LOAD_ACTION depth_load_action = LOAD_ACTION_LOAD,
            STORE_ACTION depth_store_action = STORE_ACTION_STORE,
            LOAD_ACTION stencil_load_action = LOAD_ACTION_LOAD,
            STORE_ACTION stencil_store_action = STORE_ACTION_STORE) CYBER_NOEXCEPT;

            RGRenderPass& set_pipeline(RenderObject::IRenderPipeline* pipeline) CYBER_NOEXCEPT;
            RGRenderPass& add_input(const char8_t* name, RGTextureRef texture) CYBER_NOEXCEPT;
            RGRenderPass& add_input(const char8_t* name, RGBufferRef buffer) CYBER_NOEXCEPT;

            RenderObject::IRenderPipeline* pipeline;
            eastl::map<uint32_t, RGTextureRef> render_targets;
            eastl::map<const char8_t*, RGTextureRef, Utf8StringLess> input_textures;
            eastl::map<const char8_t*, RGBufferRef, Utf8StringLess> input_buffers;
            RGRenderResource* depth_stencil = nullptr;

            LOAD_ACTION dload_action;
            STORE_ACTION dstore_action;
            LOAD_ACTION sload_action;
            STORE_ACTION sstore_action;

            eastl::vector<LOAD_ACTION> mrt_load_actions;
            eastl::vector<STORE_ACTION> mrt_store_actions;
            eastl::vector<GRAPHICS_CLEAR_VALUE> mrt_clear_values;

            uint32_t num_render_target = 0;
            render_pass_execute_function pass_function;
        };

        class CYBER_RUNTIME_API RGComputePass : public RGPass
        {
        public:
            RGComputePass();
            virtual ~RGComputePass() = default;
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
            RenderPassNode() : PassNode(RG_RENDER_PASS_NODE, RG_RENDER_PASS) {}
        };

        struct ComputePassNode : public PassNode
        {
            ComputePassNode() : PassNode(RG_COMPUTE_PASS_NODE, RG_COMPUTE_PASS) {}
        };

        struct PresentPassNode : public PassNode
        {
            PresentPassNode() : PassNode(RG_PRESENT_PASS_NODE, RG_PRESENT_PASS) {}
        };

        struct ResourceNode : public RenderGraphNode
        {
            ResourceNode(ERGObjectType type) : RenderGraphNode(type) {}

        };

        struct BufferNode : public ResourceNode
        {
            BufferNode() : ResourceNode(RG_BUFFER_NODE) {}
            RenderObject::IBuffer* buffer = nullptr;
        };
 
        struct TextureNode : public ResourceNode
        {
            TextureNode() : ResourceNode(RG_TEXTURE_NODE)
            {
            }

            RenderObject::ITexture* texture = nullptr;
        };

    }
}
