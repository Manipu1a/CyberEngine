#pragma once
#include "common/cyber_graphics_config.h"
#include "graphics_types.h"
#include "frame_buffer.h"
#include "render_device.hpp"

namespace Cyber
{
    namespace RenderObject
    {
        struct DeviceContextDesc
        {
            const char8_t* name;
            
            COMMAND_QUEUE_TYPE queue_type;

            bool is_deferrd_context = false;

            uint8_t context_id = 0;
            
            uint8_t queue_id = 0xFF;
        };

        struct CYBER_GRAPHICS_API IDeviceContext
        {
            virtual void transition_resource_state(const ResourceBarrierDesc& barrierDesc) = 0;
            // Queue APIs
            virtual IQueue* get_queue(COMMAND_QUEUE_TYPE type, uint32_t index) = 0;
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
            /// CMDS
            virtual void cmd_begin(ICommandBuffer* cmd) = 0;
            virtual void cmd_end(ICommandBuffer* cmd) = 0;
            virtual void cmd_resource_barrier(const ResourceBarrierDesc& barrierDesc) = 0;
            // Render Pass
            virtual void cmd_next_sub_pass() = 0;
            virtual void cmd_end_render_pass() = 0;
            virtual void render_encoder_bind_descriptor_set(IDescriptorSet* descriptorSet) = 0;
            virtual void render_encoder_set_viewport(float x, float y, float width, float height, float min_depth, float max_depth) = 0;
            virtual void render_encoder_set_scissor(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;
            virtual void render_encoder_bind_pipeline(IRenderPipeline* pipeline) = 0;
            virtual void render_encoder_bind_vertex_buffer(uint32_t buffer_count, IBuffer** buffers,const uint32_t* strides, const uint32_t* offsets) = 0;
            virtual void render_encoder_bind_index_buffer(IBuffer* buffer, uint32_t index_stride, uint64_t offset) = 0;
            virtual void render_encoder_push_constants(IRootSignature* rs, const char8_t* name, const void* data) = 0;
            virtual void render_encoder_draw(uint32_t vertex_count, uint32_t first_vertex) = 0;
            virtual void render_encoder_draw_instanced(uint32_t vertex_count, uint32_t first_vertex, uint32_t instance_count, uint32_t first_instance) = 0;
            virtual void render_encoder_draw_indexed(uint32_t index_count, uint32_t first_index, uint32_t first_vertex) = 0;
            virtual void render_encoder_draw_indexed_instanced(uint32_t index_count, uint32_t first_index, uint32_t instance_count, uint32_t first_instance, uint32_t first_vertex) = 0;
        };

        template<typename EngineImplTraits>
        class CYBER_GRAPHICS_API DeviceContextBase : public ObjectBase<typename EngineImplTraits::DeviceContextInterface>
        {
        public:
            using RenderDeviceImplType = typename EngineImplTraits::RenderDeviceImplType;

            DeviceContextBase(RenderDeviceImplType* device, const DeviceContextDesc& desc) : render_device(device), desc(desc) {}

            virtual void cmd_begin_render_pass(const BeginRenderPassAttribs& beginRenderPassDesc) override
            {
                m_subpassIndex = 0;
                m_pRenderPass = beginRenderPassDesc.pRenderPass;
                m_pFrameBuffer = beginRenderPassDesc.pFramebuffer;
                m_beginRenderPassAttribs = beginRenderPassDesc;
            }

            virtual void cmd_next_sub_pass() override
            {
                m_subpassIndex++;
            }
            

            bool is_deferred_context() const { return desc.is_deferrd_context; }
            
            DeviceContextIndex get_context_id() const { return desc.context_id; }

            DeviceContextIndex get_execution_context_id() const
            {
                return is_deferred_context() ? immediate_context_id : get_context_id();
            }
            
            SoftwareQueueIndex get_command_queue_id() const
            {
                return (SoftwareQueueIndex)get_execution_context_id();
            }
            
        protected:
            RenderDeviceImplType* render_device = nullptr;

            DeviceContextDesc desc;

            DeviceContextIndex immediate_context_id = 0xFF;

            uint32_t m_subpassIndex = 0;
            BeginRenderPassAttribs m_beginRenderPassAttribs;
            IRenderPass* m_pRenderPass = nullptr;
            IFrameBuffer* m_pFrameBuffer = nullptr;
        };
    }
}


