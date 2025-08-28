#pragma once
#include "common/cyber_graphics_config.h"
#include "graphics_types.h"
#include "frame_buffer.h"
#include "descriptor_set.h"
#include "render_pipeline.h"
#include "render_pass.h"

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

        struct Viewport
        {
            float top_left_x = 0.0f;
            float top_left_y = 0.0f;
            float width = 0.0f;
            float height = 0.0f;
            float min_depth = 0.0f;
            float max_depth = 1.0f;

            constexpr Viewport(float _top_left_x, float _top_left_y, 
                float _width, float _height, 
                float _min_depth = 0.0f, float _max_depth = 1.0f) noexcept
                : top_left_x(_top_left_x), 
                top_left_y(_top_left_y), 
                width(_width), 
                height(_height), 
                min_depth(_min_depth), 
                max_depth(_max_depth) {}

            constexpr Viewport() noexcept {}
        };

        struct Rect
        {
            int32_t left = 0;
            int32_t top = 0;
            int32_t right = 0;
            int32_t bottom = 0;

            constexpr Rect(int32_t _left, int32_t _top, 
                int32_t _right, int32_t _bottom) noexcept
                : left(_left), top(_top), right(_right), bottom(_bottom) {}

            constexpr Rect() noexcept {}
            
            constexpr bool is_valid() const
            {
                return (right > left) && (bottom > top);
            } 
        };

        struct BeginRenderPassAttribs
        {
            IFrameBuffer* pFramebuffer;
            IRenderPass* pRenderPass;
            uint32_t ClearValueCount;
            GRAPHICS_CLEAR_VALUE* color_clear_values = nullptr;
            GRAPHICS_CLEAR_VALUE depth_stencil_clear_value;
            RESOURCE_STATE_TRANSITION_MODE TransitionMode;
        };

        struct CYBER_GRAPHICS_API IDeviceContext
        {
            virtual void transition_resource_state(const ResourceBarrierDesc& barrierDesc) = 0;
            /// CMDS
            virtual void cmd_begin() = 0;
            virtual void cmd_end() = 0;
            virtual void cmd_resource_barrier(ITexture* texture, GRAPHICS_RESOURCE_STATE srcState, GRAPHICS_RESOURCE_STATE dstState) = 0;
            virtual void cmd_resource_barrier(IBuffer* buffer, GRAPHICS_RESOURCE_STATE srcState, GRAPHICS_RESOURCE_STATE dstState) = 0;
            virtual void cmd_resource_barrier(const ResourceBarrierDesc& barrierDesc) = 0;

            virtual void flush() = 0;
            virtual void finish_frame() = 0;
            virtual void set_frame_buffer(IFrameBuffer* frameBuffer) = 0;
            virtual IFrameBuffer* get_frame_buffer() const = 0;

            virtual void set_render_target(uint32_t numRenderTargets, ITexture_View* renderTargets[], ITexture_View* depthTarget) = 0;
            // Render Pass
            virtual void cmd_begin_render_pass(const BeginRenderPassAttribs& beginRenderPassDesc) = 0;
            virtual void cmd_next_sub_pass() = 0;
            virtual void cmd_end_render_pass() = 0;
            virtual void render_encoder_bind_descriptor_set(IDescriptorSet* descriptorSet) = 0;
            virtual void render_encoder_set_viewport(uint32_t num_viewport, const Viewport* vps) = 0;
            virtual void render_encoder_set_scissor(uint32_t num_rects, const Rect* rect) = 0;
            virtual void render_encoder_set_blend_factor(const float* blend_factor) = 0;
            virtual void render_encoder_bind_pipeline(IRenderPipeline* pipeline) = 0;
            virtual void render_encoder_bind_vertex_buffer(uint32_t buffer_count, IBuffer** buffers,const uint32_t* strides, const uint64_t* offsets) = 0;
            virtual void render_encoder_bind_index_buffer(IBuffer* buffer, uint32_t index_stride, uint64_t offset) = 0;
            virtual void render_encoder_push_constants(IRootSignature* rs, const char8_t* name, const void* data) = 0;
            virtual void render_encoder_draw(uint32_t vertex_count, uint32_t first_vertex) = 0;
            virtual void render_encoder_draw_instanced(uint32_t vertex_count, uint32_t first_vertex, uint32_t instance_count, uint32_t first_instance) = 0;
            virtual void render_encoder_draw_indexed(uint32_t index_count, uint32_t first_index, uint32_t first_vertex) = 0;
            virtual void render_encoder_draw_indexed_instanced(uint32_t index_count, uint32_t first_index, uint32_t instance_count, uint32_t first_instance, uint32_t first_vertex) = 0;

            virtual void set_shader_resource_view(SHADER_STAGE stage, uint32_t binding, ITexture_View* textureView) = 0;
            virtual void set_constant_buffer_view(SHADER_STAGE stage, uint32_t binding, IBuffer* buffer) = 0;
            virtual void set_unordered_access_view(SHADER_STAGE stage, uint32_t binding, IBuffer* buffer) = 0;
            virtual void set_root_constant_buffer_view(SHADER_STAGE stage, uint32_t binding, IBuffer* buffer) = 0;
            
            virtual void prepare_for_rendering() = 0;
            
            // Render Pass
            virtual IRenderPass* create_render_pass(const RenderPassDesc& renderPassDesc) = 0;
        };

        template<typename EngineImplTraits>
        class CYBER_GRAPHICS_API DeviceContextBase : public ObjectBase<typename EngineImplTraits::DeviceContextInterface>
        {
        public:
            using RenderDeviceImplType = typename EngineImplTraits::RenderDeviceImplType;
            using RenderPipelineImplType = typename EngineImplTraits::RenderPipelineImplType;
            DeviceContextBase(RenderDeviceImplType* device, const DeviceContextDesc& desc) : render_device(device), desc(desc) {}

            virtual void cmd_begin_render_pass(const BeginRenderPassAttribs& beginRenderPassDesc) override
            {
                m_subpassIndex = 0;
                m_pRenderPass = beginRenderPassDesc.pRenderPass;
                m_pFrameBuffer = beginRenderPassDesc.pFramebuffer;
                ClearValueCount = beginRenderPassDesc.ClearValueCount;
                color_clear_values = (GRAPHICS_CLEAR_VALUE*)cyber_malloc(ClearValueCount * sizeof(GRAPHICS_CLEAR_VALUE));
                for(uint32_t i = 0; i < ClearValueCount; ++i)
                {
                    color_clear_values[i] = beginRenderPassDesc.color_clear_values[i];
                }
                depth_stencil_clear_value = beginRenderPassDesc.depth_stencil_clear_value;
                TransitionMode = beginRenderPassDesc.TransitionMode;
            }

            virtual void cmd_next_sub_pass() override
            {
                m_subpassIndex++;
            }

            virtual void set_frame_buffer(IFrameBuffer* frameBuffer) override
            {
                m_pFrameBuffer = frameBuffer;
            }

            virtual IFrameBuffer* get_frame_buffer() const override
            {
                return m_pFrameBuffer;
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
            IRenderPass* m_pRenderPass = nullptr;
            IFrameBuffer* m_pFrameBuffer = nullptr;
            RenderPipelineImplType* render_pipeline = nullptr;
            uint32_t ClearValueCount;
            GRAPHICS_CLEAR_VALUE* color_clear_values;
            GRAPHICS_CLEAR_VALUE depth_stencil_clear_value;
            RESOURCE_STATE_TRANSITION_MODE TransitionMode;
        };
    }
}


