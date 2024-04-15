#pragma once

#include "common/cyber_graphics_config.h"
#include "interface/graphics_types.h"
#include "texture.hpp"
#include "texture_view.h"
#include "buffer.h"
#include "frame_buffer.h"
#include "render_pass.h"
#include "swap_chain.hpp"
#include "queue.h"
#include "command_buffer.h"
#include "command_pool.h"
#include "semaphore.h"
#include "fence.h"
#include "render_pipeline.h"
#include "root_signature.hpp"
#include "root_signature_pool.h"
#include "shader_library.h"
#include "shader_reflection.hpp"
#include "adapter.h"
#include "descriptor_set.h"
#include "object_base.h"


namespace Cyber
{
    namespace RenderObject
    {
        struct CYBER_GRAPHICS_API RenderDeviceCreateDesc
        {
            bool m_disablePipelineCache;
            eastl::vector<QueueGroupDesc> m_queueGroups;
            uint32_t m_queueGroupCount;
        };

        /// Defines resource state transition mode performed by various commands.
        CYBER_TYPED_ENUM(RESOURCE_STATE_TRANSITION_MODE, uint8_t)
        {
            /// Perform no state transitions and no state validation.
            /// Resource states are not accessed (either read or written) by the command.
            RESOURCE_STATE_TRANSITION_MODE_NONE = 0,
            /// Transition resources to the states required by the specific command.
            /// Resources in unknown state are ignored.
            RESOURCE_STATE_TRANSITION_MODE_TRANSITION = 1,
            /// Do not transition, but verify that states are correct.
            RESOURCE_STATE_TRANSITION_MODE_VERIFY = 2
        };

        struct BeginRenderPassAttribs
        {
            IFrameBuffer* pFramebuffer;
            IRenderPass* pRenderPass;
            uint32_t ClearValueCount;
            GRAPHICS_CLEAR_VALUE* pClearValues;
            RESOURCE_STATE_TRANSITION_MODE TransitionMode;
        };

        // Render device interface
        struct CYBER_GRAPHICS_API IRenderDevice
        {
            // interface
            virtual GRAPHICS_BACKEND get_backend() const = 0;
            virtual NVAPI_STATUS get_nvapi_status() const = 0;
            virtual AGS_RETURN_CODE get_ags_status() const = 0;

            // Instance APIs
            virtual void free_instance(IInstance* instance) = 0;
            // Device APIS
            virtual void free_device() = 0;
            // API Object APIs
            virtual Surface* surface_from_hwnd(HWND hwnd) = 0;
            virtual void free_surface(Surface* surface) = 0;
            virtual IFence* create_fence() = 0;
            virtual void wait_fences(IFence** fences, uint32_t fenceCount) = 0;
            virtual void free_fence(IFence* fence) = 0;
            virtual FENCE_STATUS query_fence_status(IFence* fence) = 0;
            virtual ISwapChain* create_swap_chain(const SwapChainDesc& swapchainDesc) = 0;
            virtual void free_swap_chain(ISwapChain* swapChain) = 0;
            virtual void enum_adapters(IInstance* instance, IAdapter** adapters, uint32_t* adapterCount) = 0;
            virtual uint32_t acquire_next_image(ISwapChain* swapChain, const AcquireNextDesc& acquireDesc) = 0;
            virtual IFrameBuffer* create_frame_buffer(const FrameBuffserDesc& frameBufferDesc) = 0;
            // Queue APIs
            virtual IQueue* get_queue(QUEUE_TYPE type, uint32_t index) = 0;
            virtual void submit_queue(IQueue* queue, const QueueSubmitDesc& submitDesc) = 0;
            virtual void present_queue(IQueue* queue, const QueuePresentDesc& presentDesc) = 0;
            virtual void wait_queue_idle(IQueue* queue) = 0;
            virtual void free_queue(IQueue* queue) = 0;
            // Command APIs
            virtual ICommandPool* create_command_pool(IQueue* queue, const CommandPoolCreateDesc& commandPoolDesc) = 0;
            virtual void reset_command_pool(ICommandPool* pool) = 0;
            virtual void free_command_pool(ICommandPool* pool) = 0;
            virtual ICommandBuffer* create_command_buffer(ICommandPool* pool, const CommandBufferCreateDesc& commandBufferDesc) = 0;
            virtual void free_command_buffer(ICommandBuffer* CommandBuffer) = 0;
            /// RootSignature
            virtual IRootSignature* create_root_signature(const RootSignatureCreateDesc& rootSigDesc) = 0;
            virtual void free_root_signature(IRootSignature* rootSignature) = 0;
            virtual IDescriptorSet* create_descriptor_set(const DescriptorSetCreateDesc& dSetDesc) = 0;
            virtual void update_descriptor_set(IDescriptorSet* set, const DescriptorData* updateDesc, uint32_t count) = 0;
            virtual IRenderPipeline* create_render_pipeline(const RenderPipelineCreateDesc& pipelineDesc) = 0;
            virtual void free_render_pipeline(IRenderPipeline* pipeline) = 0;
            // Resource APIs
            virtual ITextureView* create_texture_view(const RenderObject::TextureViewCreateDesc& viewDesc) = 0;
            virtual void free_texture_view(ITextureView* view) = 0;
            virtual ITexture* create_texture(const RenderObject::TextureCreateDesc& textureDesc) = 0;
            virtual void free_texture(ITexture* texture) = 0;
            virtual IBuffer* create_buffer(const RenderObject::BufferCreateDesc& bufferDesc) = 0;
            virtual void free_buffer(IBuffer* buffer) = 0;
            virtual void map_buffer(IBuffer* buffer, const BufferRange* range) = 0;
            virtual void unmap_buffer(IBuffer* buffer) = 0;

            // Shader
            virtual IShaderLibrary* create_shader_library(const struct ShaderLibraryCreateDesc& desc) = 0;
            virtual void free_shader_library(IShaderLibrary* shaderLibrary) = 0;
            /// CMDS
            virtual void cmd_begin(ICommandBuffer* cmd) = 0;
            virtual void cmd_end(ICommandBuffer* cmd) = 0;
            virtual void cmd_resource_barrier(ICommandBuffer* cmd, const ResourceBarrierDesc& barrierDesc) = 0;
            // Render Pass
            virtual IRenderPass* create_render_pass(const RenderPassDesc& renderPassDesc) = 0;
            virtual RenderPassEncoder* cmd_begin_render_pass(ICommandBuffer* cmd, const BeginRenderPassAttribs& beginRenderPassDesc) = 0;
            virtual void cmd_end_render_pass(ICommandBuffer* cmd) = 0;
            virtual void render_encoder_bind_descriptor_set(RenderPassEncoder* encoder, IDescriptorSet* descriptorSet) = 0;
            virtual void render_encoder_set_viewport(RenderPassEncoder* encoder, float x, float y, float width, float height, float min_depth, float max_depth) = 0;
            virtual void render_encoder_set_scissor(RenderPassEncoder* encoder, uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;
            virtual void render_encoder_bind_pipeline(RenderPassEncoder* encoder, IRenderPipeline* pipeline) = 0;
            virtual void render_encoder_bind_vertex_buffer(RenderPassEncoder* encoder, uint32_t buffer_count, IBuffer** buffers,const uint32_t* strides, const uint32_t* offsets) = 0;
            virtual void render_encoder_bind_index_buffer(RenderPassEncoder* encoder, IBuffer* buffer, uint32_t index_stride, uint64_t offset) = 0;
            virtual void render_encoder_push_constants(RenderPassEncoder* encoder, IRootSignature* rs, const char8_t* name, const void* data) = 0;
            virtual void render_encoder_draw(RenderPassEncoder* encoder, uint32_t vertex_count, uint32_t first_vertex) = 0;
            virtual void render_encoder_draw_instanced(RenderPassEncoder* encoder, uint32_t vertex_count, uint32_t first_vertex, uint32_t instance_count, uint32_t first_instance) = 0;
            virtual void render_encoder_draw_indexed(RenderPassEncoder* encoder, uint32_t index_count, uint32_t first_index, uint32_t first_vertex) = 0;
            virtual void render_encoder_draw_indexed_instanced(RenderPassEncoder* encoder, uint32_t index_count, uint32_t first_index, uint32_t instance_count, uint32_t first_instance, uint32_t first_vertex) = 0;
        };

        template<typename EngineImplTraits>
        class CYBER_GRAPHICS_API RenderDeviceBase : public ObjectBase<typename EngineImplTraits::RenderDeviceInterface>
        {
        public:
            using TextureImplType = typename EngineImplTraits::TextureImplType;
            using TextureViewImplType = typename EngineImplTraits::TextureViewImplType;
            using BufferImplType = typename EngineImplTraits::BufferImplType;
            using RenderDeviceInterface = typename EngineImplTraits::RenderDeviceInterface;
            using TRenderDeviceBase = ObjectBase<RenderDeviceInterface>;

            RenderDeviceBase(IAdapter* adapter, const RenderDeviceCreateDesc& deviceDesc) : TRenderDeviceBase(), m_desc(deviceDesc)
            {
                this->m_pAdapter = adapter;
                
                create_render_device_impl(adapter, deviceDesc);
            }
            virtual ~RenderDeviceBase()
            {
            }
        protected:
            virtual void create_render_device_impl(IAdapter* adapter, const RenderDeviceCreateDesc& deviceDesc) = 0;
        protected:
            IAdapter* m_pAdapter;
            RenderDeviceCreateDesc m_desc;

        public:
            friend TextureImplType;
            friend TextureViewImplType;
            friend BufferImplType;
        };
    }
}