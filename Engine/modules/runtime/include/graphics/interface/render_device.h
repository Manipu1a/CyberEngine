#pragma once

#include "common/cyber_graphics_config.h"
#include "interface/rhi.h"
#include "texture.h"
#include "texture_view.h"
#include "buffer.h"
#include "frame_buffer.h"
#include "render_pass.h"

namespace Cyber
{
    namespace RenderObject
    {
        // Render device interface
        struct CYBER_GRAPHICS_API IRenderDevice
        {

        };

        struct CYBER_GRAPHICS_API RenderDeviceCreateDesc
        {
            bool bDisablePipelineCache;
            eastl::vector<RHIQueueGroupDesc> queue_groups;
            uint32_t queue_group_count;
        };
        
        template<typename EngineImplTraits>
        class CYBER_GRAPHICS_API CERenderDevice : public RenderObjectBase<typename EngineImplTraits::RenderDeviceInterface, typename EngineImplTraits::RenderDeviceImplType>
        {
        public:
            using TextureImplType = typename EngineImplTraits::TextureImplType;
            using TextureViewImplType = typename EngineImplTraits::TextureViewImplType;
            using BufferImplType = typename EngineImplTraits::BufferImplType;
            using RenderDeviceImplType = typename EngineImplTraits::RenderDeviceImplType;

            CERenderDevice(RHIAdapter* adapter, const RenderDeviceCreateDesc& deviceDesc)
            {
                this->adapter = adapter;
                this->device_desc = deviceDesc;
            }
            virtual ~CERenderDevice()
            {
                free_device();
            }
        private:
            CERenderDevice();
            
        protected:
            RHIAdapter* adapter;
            RenderDeviceCreateDesc device_desc;
        public:
            // Instance APIs
            virtual RHIInstance* create_instance(const RHIInstanceCreateDesc& instanceDesc) 
            {
                cyber_core_assert(false, "Empty implement create_instance!");
                return nullptr;
            }
            virtual void free_instance(RHIInstance* instance)
            {
                cyber_core_assert(false, "Empty implement free_instance!");
            }
            // Device APIS
            virtual void create_device(RHIAdapter* pAdapter, const RenderDeviceCreateDesc& deviceDesc) 
            {
                cyber_core_assert(false, "Empty implement create_device!");
            }
            virtual void free_device() 
            {
                cyber_core_assert(false, "Empty implement free_device!");
            }
            // API Object APIs
            virtual RHISurface* surface_from_hwnd(HWND hwnd)
            {
                cyber_core_assert(false, "Empty implement surface_from_hwnd!");
                return nullptr;
            }
            virtual void free_surface(RHISurface* pSurface)
            {
                cyber_core_assert(false, "Empty implement free_surface!");
            }
            virtual RHIFence* create_fence() 
            {
                cyber_core_assert(false, "Empty implement create_fence!");
                return nullptr;
            };
            virtual void wait_fences(RHIFence** fences, uint32_t fenceCount)
            {
                cyber_core_assert(false, "Empty implement wait_fences!");
            }
            virtual void free_fence(RHIFence* fence)
            {
                cyber_core_assert(false, "Empty implement free_fence!");
            }
            virtual ERHIFenceStatus query_fence_status(RHIFence* pFence)
            {
                cyber_core_assert(false, "Empty implement query_fence_status!");
                return RHI_FENCE_STATUS_NOTSUBMITTED;
            }
            virtual RHISwapChain* create_swap_chain(const RHISwapChainCreateDesc& swapchainDesc)
            {
                cyber_core_assert(false, "Empty implement create_swap_chain!");
                return nullptr;
            }
            virtual void free_swap_chain(RHISwapChain* pSwapChain)
            {
                cyber_core_assert(false, "Empty implement free_swap_chain!");
            }
            virtual void enum_adapters(RHIInstance* instance, RHIAdapter** adapters, uint32_t* adapterCount)
            {
                cyber_core_assert(false, "Empty implement enum_adapters!");
            }
            virtual uint32_t acquire_next_image(RHISwapChain* pSwapChain, const RHIAcquireNextDesc& acquireDesc)
            {
                cyber_core_assert(false, "Empty implement acquire_next_image!");
                return 0;
            }
            virtual CEFrameBuffer* create_frame_buffer(const FrameBuffserDesc& frameBufferDesc)
            {
                cyber_core_assert(false, "Empty implement create_frame_buffer!");
                return nullptr;
            }
            // Queue APIs
            virtual RHIQueue* get_queue(ERHIQueueType type, uint32_t index) 
            { 
                cyber_core_assert(false, "Empty implement get_queue!");
                return nullptr;
            }
            virtual void submit_queue(RHIQueue* queue, const RHIQueueSubmitDesc& submitDesc)
            {
                cyber_core_assert(false, "Empty implement submit_queue!");
            }
            virtual void present_queue(RHIQueue* queue, const RHIQueuePresentDesc& presentDesc)
            {
                cyber_core_assert(false, "Empty implement present_queue!");
            }
            virtual void wait_queue_idle(RHIQueue* queue)
            {
                cyber_core_assert(false, "Empty implement wait_queue_idle!");
            }
            virtual void free_queue(RHIQueue* queue)
            {
                cyber_core_assert(false, "Empty implement free_queue!");
            }
            // Command APIs
            virtual RHICommandPool* create_command_pool(RHIQueue* pQueue, const CommandPoolCreateDesc& commandPoolDesc)
            {
                cyber_core_assert(false, "Empty implement create_command_pool!");
                return nullptr;
            }
            virtual void reset_command_pool(RHICommandPool* pPool)
            {
                cyber_core_assert(false, "Empty implement reset_command_pool!");
            }
            virtual void free_command_pool(RHICommandPool* pPool)
            {
                cyber_core_assert(false, "Empty implement free_command_pool!");
            }
            virtual RHICommandBuffer* create_command_buffer(RHICommandPool* pPool, const CommandBufferCreateDesc& commandBufferDesc)
            {
                cyber_core_assert(false, "Empty implement create_command_buffer!");
                return nullptr;
            }
            virtual void free_command_buffer(RHICommandBuffer* pCommandBuffer)
            {
                cyber_core_assert(false, "Empty implement free_command_buffer!");
            }
            /// RootSignature
            virtual RHIRootSignature* create_root_signature(const RHIRootSignatureCreateDesc& rootSigDesc)
            {
                cyber_core_assert(false, "Empty implement create_root_signature!");
                return nullptr;
            }
            virtual void free_root_signature(RHIRootSignature* pRootSignature)
            {
                cyber_core_assert(false, "Empty implement free_root_signature!");
            }
            virtual RHIDescriptorSet* create_descriptor_set(const RHIDescriptorSetCreateDesc& dSetDesc)
            {
                cyber_core_assert(false, "Empty implement create_descriptor_set!");
                return nullptr;
            }
            virtual void update_descriptor_set(RHIDescriptorSet* set, const RHIDescriptorData* updateDesc, uint32_t count)
            {
                cyber_core_assert(false, "Empty implement update_descriptor_set!");
            }
            virtual RHIRenderPipeline* create_render_pipeline(const RHIRenderPipelineCreateDesc& pipelineDesc)
            {
                cyber_core_assert(false, "Empty implement create_render_pipeline!");
                return nullptr;
            }
            virtual void free_render_pipeline(RHIRenderPipeline* pipeline)
            {
                cyber_core_assert(false, "Empty implement free_render_pipeline!");
            }

            // Resource APIs
            virtual TextureViewImplType* create_texture_view(const RenderObject::TextureViewCreateDesc& viewDesc)
            {
                cyber_core_assert(false, "Empty implement create_texture_view!");
                return nullptr;
            }
            virtual void free_texture_view(TextureViewImplType* view)
            {
                cyber_core_assert(false, "Empty implement free_texture_view!");
            }
            virtual TextureImplType* create_texture(const RenderObject::TextureCreateDesc& textureDesc) 
            {
                cyber_core_assert(false, "Empty implement create_texture!");
                return nullptr;
            }
            virtual void free_texture(TextureImplType* texture)
            {
                cyber_core_assert(false, "Empty implement free_texture!");
            }
            virtual BufferImplType* create_buffer(const RenderObject::BufferCreateDesc& bufferDesc) 
            {
                cyber_core_assert(false, "Empty implement create_buffer!");
                return nullptr;
            }
            virtual void free_buffer(BufferImplType* buffer)
            {
                cyber_core_assert(false, "Empty implement free_buffer!");
            }
            virtual void map_buffer(BufferImplType* buffer, const RHIBufferRange* range)
            {
                cyber_core_assert(false, "Empty implement map_buffer!");
            }
            virtual void unmap_buffer(BufferImplType* buffer)
            {
                cyber_core_assert(false, "Empty implement unmap_buffer!");
            }

            // Shader
            virtual RHIShaderLibrary* create_shader_library(const struct RHIShaderLibraryCreateDesc& desc)
            {
                cyber_core_assert(false, "Empty implement create_shader_library!");
                return nullptr;
            }
            virtual void free_shader_library(RHIShaderLibrary* shaderLibrary)
            {
                cyber_core_assert(false, "Empty implement free_shader_library!");
            }

            /// CMDS
            virtual void cmd_begin(RHICommandBuffer* cmd)
            {
                cyber_core_assert(false, "Empty implement cmd_begin!");
            }
            virtual void cmd_end(RHICommandBuffer* cmd)
            {
                cyber_core_assert(false, "Empty implement cmd_end!");
            }
            virtual void cmd_resource_barrier(RHICommandBuffer* cmd, const RHIResourceBarrierDesc& barrierDesc)
            {
                cyber_core_assert(false, "Empty implement cmd_resource_barrier!");
            }
            // Render Pass
            virtual CERenderPass* create_render_pass(const RenderPassDesc& renderPassDesc)
            {
                cyber_core_assert(false, "Empty implement create_render_pass!");
                return nullptr;
            }

            virtual RHIRenderPassEncoder* cmd_begin_render_pass(RHICommandBuffer* cmd, const RenderPassDesc& beginRenderPassDesc)
            {
                cyber_core_assert(false, "Empty implement cmd_begin_render_pass!");
                return nullptr;
            }
            virtual void cmd_end_render_pass(RHICommandBuffer* cmd)
            {
                cyber_core_assert(false, "Empty implement cmd_end_render_pass!");
            }
            virtual void render_encoder_bind_descriptor_set(RHIRenderPassEncoder* encoder, RHIDescriptorSet* descriptorSet)
            {
                cyber_core_assert(false, "Empty implement render_encoder_bind_descriptor_set!");
            }
            virtual void render_encoder_set_viewport(RHIRenderPassEncoder* encoder, float x, float y, float width, float height, float min_depth, float max_depth)
            {
                cyber_core_assert(false, "Empty implement render_encoder_set_viewport!");
            }
            virtual void render_encoder_set_scissor(RHIRenderPassEncoder* encoder, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
            {
                cyber_core_assert(false, "Empty implement render_encoder_set_scissor!");
            }
            virtual void render_encoder_bind_pipeline(RHIRenderPassEncoder* encoder, RHIRenderPipeline* pipeline)
            {
                cyber_core_assert(false, "Empty implement render_encoder_bind_pipeline!");
            }
            virtual void render_encoder_bind_vertex_buffer(RHIRenderPassEncoder* encoder, uint32_t buffer_count, BufferImplType** buffers,const uint32_t* strides, const uint32_t* offsets)
            {
                cyber_core_assert(false, "Empty implement render_encoder_bind_vertex_buffer!");
            }
            virtual void render_encoder_bind_index_buffer(RHIRenderPassEncoder* encoder, BufferImplType* buffer, uint32_t index_stride, uint64_t offset)
            {
                cyber_core_assert(false, "Empty implement render_encoder_bind_index_buffer!");
            }
            virtual void render_encoder_push_constants(RHIRenderPassEncoder* encoder, RHIRootSignature* rs, const char8_t* name, const void* data)
            {
                cyber_core_assert(false, "Empty implement render_encoder_push_constants!");
            }
            virtual void render_encoder_draw(RHIRenderPassEncoder* encoder, uint32_t vertex_count, uint32_t first_vertex)
            {
                cyber_core_assert(false, "Empty implement render_encoder_draw!");
            }
            virtual void render_encoder_draw_instanced(RHIRenderPassEncoder* encoder, uint32_t vertex_count, uint32_t first_vertex, uint32_t instance_count, uint32_t first_instance)
            {
                cyber_core_assert(false, "Empty implement render_encoder_draw_instanced!");
            }
            virtual void render_encoder_draw_indexed(RHIRenderPassEncoder* encoder, uint32_t index_count, uint32_t first_index, uint32_t first_vertex)
            {
                cyber_core_assert(false, "Empty implement render_encoder_draw_indexed!");
            }
            virtual void render_encoder_draw_indexed_instanced(RHIRenderPassEncoder* encoder, uint32_t index_count, uint32_t first_index, uint32_t instance_count, uint32_t first_instance, uint32_t first_vertex)
            {
                cyber_core_assert(false, "Empty implement render_encoder_draw_indexed_instanced!");
            }

            friend TextureImplType;
            friend TextureViewImplType;
            friend BufferImplType;
        };
    }
}