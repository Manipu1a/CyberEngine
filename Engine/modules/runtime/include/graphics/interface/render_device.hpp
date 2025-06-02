#pragma once

#include "common/cyber_graphics_config.h"
#include "interface/graphics_types.h"
#include "texture.hpp"
#include "texture_view.h"
#include "buffer.h"
#include "frame_buffer.h"
#include "swap_chain.hpp"
#include "command_queue.h"
#include "command_buffer.h"
#include "fence.h"
#include "root_signature.hpp"
#include "shader_library.h"
#include "sampler.h"
#include "adapter.h"
#include "object_base.h"
#include "device_context.h"
#include "EASTL/map.h"
//#include <map>


namespace Cyber
{
    namespace RenderObject
    {
        struct CYBER_GRAPHICS_API RenderDeviceCreateDesc
        {
            bool m_disablePipelineCache;
            uint32_t command_queue_count;
        };

        // Render device interface
        struct CYBER_GRAPHICS_API IRenderDevice
        {
            virtual void initialize_render_device() = 0;
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
            virtual void signal_fence(SoftwareQueueIndex command_queue_id, uint64_t value) = 0;
            virtual void wait_fences(SoftwareQueueIndex command_queue_id) = 0;
            virtual void free_fence(IFence* fence) = 0;
            virtual FENCE_STATUS query_fence_status(IFence* fence) = 0;
            virtual ISwapChain* create_swap_chain(const SwapChainDesc& swapchainDesc) = 0;
            virtual void free_swap_chain(ISwapChain* swapChain) = 0;
            virtual uint32_t acquire_next_image(ISwapChain* swapChain, const AcquireNextDesc& acquireDesc) = 0;
            virtual IFrameBuffer* create_frame_buffer(const FrameBuffserDesc& frameBufferDesc) = 0;
            
            // Queue APIs
            virtual void present(ISwapChain* swap_chain) = 0;
            virtual void wait_queue_idle(ICommandQueue* queue) = 0;
            virtual void free_queue(ICommandQueue* queue) = 0;

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

            virtual ITexture* create_texture(const RenderObject::TextureCreateDesc& textureDesc, TextureData* data = nullptr) = 0;
            virtual void free_texture(ITexture* texture) = 0;
            virtual IBuffer* create_buffer(const RenderObject::BufferCreateDesc& bufferDesc, BufferData* initial_data = nullptr) = 0;
            virtual void free_buffer(IBuffer* buffer) = 0;
            virtual void* map_buffer(IBuffer* buffer, MAP_TYPE map_type, MAP_FLAGS map_flags) = 0;
            virtual void unmap_buffer(IBuffer* buffer, MAP_TYPE map_type) = 0;
            virtual ISampler* create_sampler(const RenderObject::SamplerCreateDesc& samplerDesc) = 0;
            // Shader
            virtual IShaderLibrary* create_shader_library(const struct ShaderLibraryCreateDesc& desc) = 0;
            virtual void free_shader_library(IShaderLibrary* shaderLibrary) = 0;
        };

        template<typename EngineImplTraits>
        class CYBER_GRAPHICS_API RenderDeviceBase : public ObjectBase<typename EngineImplTraits::RenderDeviceInterface>
        {
        public:
            using TextureImplType = typename EngineImplTraits::TextureImplType;
            using TextureViewImplType = typename EngineImplTraits::TextureViewImplType;
            using BufferImplType = typename EngineImplTraits::BufferImplType;
            using DeviceContextImplType = typename EngineImplTraits::DeviceContextImplType;
            using RenderDeviceInterface = typename EngineImplTraits::RenderDeviceInterface;
            using CommandQueueImplType = typename EngineImplTraits::CommandQueueImplType;
            using TRenderDeviceBase = ObjectBase<RenderDeviceInterface>;

            RenderDeviceBase(IAdapter* adapter, const RenderDeviceCreateDesc& deviceDesc) : TRenderDeviceBase(), m_desc(deviceDesc)
            {
                this->m_pAdapter = adapter;
            }
            
            virtual void initialize_render_device() override
            {
                create_render_device_impl();
            }

            virtual ~RenderDeviceBase()
            {
                
            }

            void set_device_context(size_t ctx_id, DeviceContextImplType* device_context)
            {
                m_deviceContexts[ctx_id] = device_context;
            }

            DeviceContextImplType* get_device_context(uint32_t index) const
            {
                return m_deviceContexts[index];
            }

        protected:
            virtual void create_render_device_impl() = 0;
            
        protected:
            IAdapter* m_pAdapter;
            RenderDeviceCreateDesc m_desc;

            eastl::vector<DeviceContextImplType*> m_deviceContexts;
            eastl::vector<CommandQueueImplType*> m_commandQueues;
        public:
            friend TextureImplType;
            friend TextureViewImplType;
            friend BufferImplType;
        };
    }
}