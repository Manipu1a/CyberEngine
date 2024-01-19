#pragma once

#include "common/cyber_graphics_config.h"
#include "interface/rhi.h"
#include "texture.hpp"
#include "texture_view.h"
#include "buffer.h"
#include "frame_buffer.h"
#include "render_pass.h"
#include "swap_chain.h"
#include "queue.h"
#include "command_buffer.h"
#include "command_pool.h"
#include "semaphore.h"
#include "fence.h"

namespace Cyber
{
    namespace RenderObject
    {
        struct CYBER_GRAPHICS_API RenderDeviceCreateDesc
        {
            bool bDisablePipelineCache;
            eastl::vector<RHIQueueGroupDesc> queue_groups;
            uint32_t queue_group_count;
        };
        
        // Render device interface
        struct CYBER_GRAPHICS_API IRenderDevice
        {
            // Instance APIs
            virtual RHIInstance* create_instance(const RHIInstanceCreateDesc& instanceDesc) = 0;
            virtual void free_instance(RHIInstance* instance) = 0;
            // Device APIS
            virtual void create_device(RHIAdapter* pAdapter, const RenderDeviceCreateDesc& deviceDesc) = 0;
            virtual void free_device() = 0;
            // API Object APIs
            virtual RHISurface* surface_from_hwnd(HWND hwnd) = 0;
            virtual void free_surface(RHISurface* pSurface) = 0;
            virtual IFence* create_fence() = 0;
            virtual void wait_fences(IFence** fences, uint32_t fenceCount) = 0;
            virtual void free_fence(IFence* fence) = 0;
            virtual ERHIFenceStatus query_fence_status(IFence* pFence) = 0;
            virtual ISwapChain* create_swap_chain(const SwapChainDesc& swapchainDesc) = 0;
            virtual void free_swap_chain(ISwapChain* pSwapChain) = 0;
            virtual void enum_adapters(RHIInstance* instance, RHIAdapter** adapters, uint32_t* adapterCount) = 0;
            virtual uint32_t acquire_next_image(ISwapChain* pSwapChain, const RHIAcquireNextDesc& acquireDesc) = 0;
            virtual IFrameBuffer* create_frame_buffer(const FrameBuffserDesc& frameBufferDesc) = 0;
            // Queue APIs
            virtual IQueue* get_queue(ERHIQueueType type, uint32_t index) = 0;
            virtual void submit_queue(IQueue* queue, const QueueSubmitDesc& submitDesc) = 0;
            virtual void present_queue(IQueue* queue, const QueuePresentDesc& presentDesc) = 0;
            virtual void wait_queue_idle(IQueue* queue) = 0;
            virtual void free_queue(IQueue* queue) = 0;
            // Command APIs
            virtual ICommandPool* create_command_pool(IQueue* pQueue, const CommandPoolCreateDesc& commandPoolDesc) = 0;
            virtual void reset_command_pool(ICommandPool* pPool) = 0;
            virtual void free_command_pool(ICommandPool* pPool) = 0;
            virtual ICommandBuffer* create_command_buffer(ICommandPool* pPool, const CommandBufferCreateDesc& commandBufferDesc) = 0;
            virtual void free_command_buffer(ICommandBuffer* pCommandBuffer) = 0;
            /// RootSignature
            virtual RHIRootSignature* create_root_signature(const RHIRootSignatureCreateDesc& rootSigDesc) = 0;
            virtual void free_root_signature(RHIRootSignature* pRootSignature) = 0;
            virtual RHIDescriptorSet* create_descriptor_set(const RHIDescriptorSetCreateDesc& dSetDesc) = 0;
            virtual void update_descriptor_set(RHIDescriptorSet* set, const RHIDescriptorData* updateDesc, uint32_t count) = 0;
            virtual RHIRenderPipeline* create_render_pipeline(const RHIRenderPipelineCreateDesc& pipelineDesc) = 0;
            virtual void free_render_pipeline(RHIRenderPipeline* pipeline) = 0;
            // Resource APIs
            virtual ITextureView* create_texture_view(const RenderObject::TextureViewCreateDesc& viewDesc) = 0;
            virtual void free_texture_view(ITextureView* view) = 0;
            virtual ITexture* create_texture(const RenderObject::TextureCreateDesc& textureDesc) = 0;
            virtual void free_texture(ITexture* texture) = 0;
            virtual IBuffer* create_buffer(const RenderObject::BufferCreateDesc& bufferDesc) = 0;
            virtual void free_buffer(IBuffer* buffer) = 0;
            virtual void map_buffer(IBuffer* buffer, const RHIBufferRange* range) = 0;
            virtual void unmap_buffer(IBuffer* buffer) = 0;

            // Shader
            virtual RHIShaderLibrary* create_shader_library(const struct RHIShaderLibraryCreateDesc& desc) = 0;
            virtual void free_shader_library(RHIShaderLibrary* shaderLibrary) = 0;
            /// CMDS
            virtual void cmd_begin(ICommandBuffer* cmd) = 0;
            virtual void cmd_end(ICommandBuffer* cmd) = 0;
            virtual void cmd_resource_barrier(ICommandBuffer* cmd, const RHIResourceBarrierDesc& barrierDesc) = 0;
            // Render Pass
            virtual IRenderPass* create_render_pass(const RenderPassDesc& renderPassDesc) = 0;
            virtual RHIRenderPassEncoder* cmd_begin_render_pass(ICommandBuffer* cmd, const RenderPassDesc& beginRenderPassDesc) = 0;
            virtual void cmd_end_render_pass(ICommandBuffer* cmd) = 0;
            virtual void render_encoder_bind_descriptor_set(RHIRenderPassEncoder* encoder, RHIDescriptorSet* descriptorSet) = 0;
            virtual void render_encoder_set_viewport(RHIRenderPassEncoder* encoder, float x, float y, float width, float height, float min_depth, float max_depth) = 0;
            virtual void render_encoder_set_scissor(RHIRenderPassEncoder* encoder, uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;
            virtual void render_encoder_bind_pipeline(RHIRenderPassEncoder* encoder, RHIRenderPipeline* pipeline) = 0;
            virtual void render_encoder_bind_vertex_buffer(RHIRenderPassEncoder* encoder, uint32_t buffer_count, IBuffer** buffers,const uint32_t* strides, const uint32_t* offsets) = 0;
            virtual void render_encoder_bind_index_buffer(RHIRenderPassEncoder* encoder, IBuffer* buffer, uint32_t index_stride, uint64_t offset) = 0;
            virtual void render_encoder_push_constants(RHIRenderPassEncoder* encoder, RHIRootSignature* rs, const char8_t* name, const void* data) = 0;
            virtual void render_encoder_draw(RHIRenderPassEncoder* encoder, uint32_t vertex_count, uint32_t first_vertex) = 0;
            virtual void render_encoder_draw_instanced(RHIRenderPassEncoder* encoder, uint32_t vertex_count, uint32_t first_vertex, uint32_t instance_count, uint32_t first_instance) = 0;
            virtual void render_encoder_draw_indexed(RHIRenderPassEncoder* encoder, uint32_t index_count, uint32_t first_index, uint32_t first_vertex) = 0;
            virtual void render_encoder_draw_indexed_instanced(RHIRenderPassEncoder* encoder, uint32_t index_count, uint32_t first_index, uint32_t instance_count, uint32_t first_instance, uint32_t first_vertex) = 0;
        };

        template<typename EngineImplTraits>
        class CYBER_GRAPHICS_API RenderDeviceBase : public RenderObjectBase<typename EngineImplTraits::RenderDeviceInterface, typename EngineImplTraits::RenderDeviceImplType>
        {
        public:
            using TRenderObjectBase = RenderObjectBase<typename EngineImplTraits::RenderDeviceInterface, typename EngineImplTraits::RenderDeviceImplType>;
            using TextureImplType = typename EngineImplTraits::TextureImplType;
            using TextureViewImplType = typename EngineImplTraits::TextureViewImplType;
            using BufferImplType = typename EngineImplTraits::BufferImplType;
            using RenderDeviceImplType = typename EngineImplTraits::RenderDeviceImplType;

            RenderDeviceBase(RHIAdapter* adapter, const RenderDeviceCreateDesc& deviceDesc) : TRenderObjectBase(nullptr)
            {
                this->adapter = adapter;
                this->device_desc = deviceDesc;
            }
            virtual ~RenderDeviceBase()
            {
            }
        private:
            RenderDeviceBase() = default;
            
        protected:
            RHIAdapter* adapter;
            RenderDeviceCreateDesc device_desc;

        public:
            friend TextureImplType;
            friend TextureViewImplType;
            friend BufferImplType;
        };
    }
}