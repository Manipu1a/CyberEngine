#pragma once
#include "d3d12.config.h"
#include "interface/render_device.hpp"
#include "backend/d3d12/graphics_types_d3d12.h"
#include "graphics/backend/d3d12/command_list_manager.h"
#include "engine_impl_traits_d3d12.hpp"

namespace Cyber
{
    namespace RenderObject
    {
        class DescriptorHeap_D3D12;
        //class CommandListManager;
        class CommandContext;

        // Render device interface
        struct CYBER_GRAPHICS_API IRenderDevice_D3D12 : public IRenderDevice
        {
            
        };

        class RenderDevice_D3D12_Impl : public RenderDeviceBase<EngineD3D12ImplTraits>
        {
        public:
            using TRenderDeviceBase = RenderDeviceBase<EngineD3D12ImplTraits>;
            using TexureImplType = EngineD3D12ImplTraits::TextureImplType;
            using BufferImplType = EngineD3D12ImplTraits::BufferImplType;
            using TextureViewImplType = EngineD3D12ImplTraits::TextureViewImplType;
            using RenderDeviceImplType = EngineD3D12ImplTraits::RenderDeviceImplType;

            RenderDevice_D3D12_Impl(IAdapter* adapter, const RenderDeviceCreateDesc& deviceDesc);

            virtual ~RenderDevice_D3D12_Impl();

        protected:
            virtual void create_render_device_impl() override;

        public:
            // Device interface
            virtual GRAPHICS_BACKEND get_backend() const override
            {
                return GRAPHICS_BACKEND_D3D12;
            }
            virtual NVAPI_STATUS get_nvapi_status() const override
            {
                return NVAPI_OK;
            }
            virtual AGS_RETURN_CODE get_ags_status() const override
            {
                return AGS_SUCCESS;
            }

            // Device APIs
            virtual void free_device() override;
            // API Object APIs
            virtual Surface* surface_from_hwnd(HWND hwnd) override;
            virtual void free_surface(Surface* surface) override;
            virtual IFence* create_fence() override;
            virtual void wait_fences(IFence** fences, uint32_t fenceCount) override;
            virtual void free_fence(IFence* fence) override;
            virtual FENCE_STATUS query_fence_status(IFence* fence) override;
            virtual ISwapChain* create_swap_chain(const SwapChainDesc& swapchainDesc) override;
            virtual void free_swap_chain(ISwapChain* swapchain) override;
            virtual uint32_t acquire_next_image(ISwapChain* swapchain, const AcquireNextDesc& acquireDesc) override;
            virtual IFrameBuffer* create_frame_buffer(const FrameBuffserDesc& frameBufferDesc) override;
            virtual ISampler* create_sampler(const RenderObject::SamplerCreateDesc& samplerDesc) override;
            virtual ICommandPool* create_command_pool(IQueue* queue, const CommandPoolCreateDesc& commandPoolDesc) override;
            virtual ICommandBuffer* create_command_buffer(ICommandPool* pool, const CommandBufferCreateDesc& commandBufferDesc) override;

            virtual IQueue* get_queue(COMMAND_QUEUE_TYPE type, uint32_t index) override;
            virtual void submit_queue(IQueue* queue, const QueueSubmitDesc& submitDesc) override;
            virtual void present_queue(IQueue* queue, const QueuePresentDesc& presentDesc) override;
            virtual void wait_queue_idle(IQueue* queue) override;
            virtual void free_queue(IQueue* queue) override;

            virtual void set_render_target(ICommandBuffer* commandBuffer, uint32_t numRenderTargets, ITextureView* renderTargets[], ITextureView* depthTarget) override;

            virtual IRootSignature* create_root_signature(const RootSignatureCreateDesc& rootSigDesc) override;
            virtual void free_root_signature(IRootSignature* rootSignature) override;
            virtual IDescriptorSet* create_descriptor_set(const DescriptorSetCreateDesc& dSetDesc) override;
            virtual void update_descriptor_set(IDescriptorSet* set, const DescriptorData* updateDesc, uint32_t count) override;

            virtual IRenderPipeline* create_render_pipeline(const RenderPipelineCreateDesc& pipelineDesc) override;
            virtual void free_render_pipeline(IRenderPipeline* pipeline) override;
            virtual void free_instance(IInstance* instance) override;

            virtual ITextureView* create_texture_view(const RenderObject::TextureViewCreateDesc& viewDesc) override;
            virtual void free_texture_view(ITextureView* view) override;
            virtual ITexture* create_texture(const RenderObject::TextureCreateDesc& textureDesc, TextureData* data = nullptr) override;
            virtual void free_texture(ITexture* texture) override;
            virtual IBuffer* create_buffer(const RenderObject::BufferCreateDesc& bufferDesc, BufferData* initial_data = nullptr) override;
            virtual void free_buffer(IBuffer* buffer) override;

            virtual void* map_buffer(IBuffer* buffer, MAP_TYPE map_type, MAP_FLAGS map_flags) override;
            virtual void unmap_buffer(IBuffer* buffer, MAP_TYPE map_type) override;

            virtual IShaderLibrary* create_shader_library(const struct ShaderLibraryCreateDesc& desc) override;
            virtual void free_shader_library(IShaderLibrary* shaderLibrary) override;
            
            virtual IRenderPass* create_render_pass(const RenderPassDesc& renderPassDesc) override;

            // create view
            void create_constant_buffer_view(const D3D12_CONSTANT_BUFFER_VIEW_DESC* desc, D3D12_CPU_DESCRIPTOR_HANDLE destHandle);
            void create_shader_resource_view(ID3D12Resource* resource, const D3D12_SHADER_RESOURCE_VIEW_DESC* desc, D3D12_CPU_DESCRIPTOR_HANDLE destHandle);
            void create_depth_stencil_view(ID3D12Resource* resource, const D3D12_DEPTH_STENCIL_VIEW_DESC* desc, D3D12_CPU_DESCRIPTOR_HANDLE destHandle);
            void create_unordered_access_view(ID3D12Resource* resource, ID3D12Resource* counterResource, const D3D12_UNORDERED_ACCESS_VIEW_DESC* desc, D3D12_CPU_DESCRIPTOR_HANDLE destHandle);
            void create_render_target_view(ID3D12Resource* resource, const D3D12_RENDER_TARGET_VIEW_DESC* desc, D3D12_CPU_DESCRIPTOR_HANDLE destHandle);

            ID3D12Device* GetD3D12Device() { return m_pDxDevice; }
            class D3D12MA::Allocator* GetD3D12ResourceAllocator() { return m_pResourceAllocator; }
            class DescriptorHeap_D3D12* GetCPUDescriptorHeaps(uint32_t heap_type) { return m_cpuDescriptorHeaps[heap_type]; }

            eastl::map<uint32_t, DescriptorHeap_D3D12*>& GetSamplerHeaps() { return m_samplerHeaps; }
            eastl::map<uint32_t, DescriptorHeap_D3D12*>& GetCbvSrvUavHeaps() { return m_cbvSrvUavHeaps; }
            struct EmptyDescriptors_D3D12* GetNullDescriptors() { return m_pNullDescriptors; }
            eastl::map<uint32_t, ID3D12CommandQueue**>& GetCommandQueues() { return m_commandQueues; }
            eastl::map<uint32_t, uint32_t>& GetCommandQueueCounts() { return m_commandQueueCounts; }
            IDXGIFactory6* GetDXGIFactory() { return m_pDXGIFactory; }
            IDXGIAdapter4* GetDXActiveGPU() { return m_pDxActiveGPU; }
            ID3D12PipelineLibrary* GetPipelineLibrary() { return m_pPipelineLibrary; }
            void* GetPSOCacheData() { return m_pPSOCacheData; }
            ID3D12Debug* GetDxDebug() { return m_pDxDebug; }
            
            void create_dma_allocallor(RenderObject::Adapter_D3D12_Impl* adapter);
            void commit_subpass_rendertargets(RenderPassEncoder* encoder);

            CommandContext* allocate_command_context(SoftwareQueueIndex command_queue_id);
        private:
            CYBER_FORCE_INLINE CommandListManager& get_command_list_manager(uint8_t index)
            {
                return cmd_list_managers[index];
            }

            HRESULT hook_CheckFeatureSupport(D3D12_FEATURE pFeature, void* pFeatureSupportData, UINT pFeatureSupportDataSize);
            HRESULT hook_CreateCommittedResource(const D3D12_HEAP_PROPERTIES* pHeapProperties, D3D12_HEAP_FLAGS HeapFlags, const D3D12_RESOURCE_DESC* pDesc, D3D12_RESOURCE_STATES InitialResourceState, const D3D12_CLEAR_VALUE* pOptimizedClearValue, REFIID riidResource, void **ppvResource);

            class D3D12MA::Allocator* m_pResourceAllocator;

            CommandListManager cmd_list_managers[3];
            // API specific descriptor heap and memory allocator
            eastl::map<uint32_t, DescriptorHeap_D3D12*> m_cpuDescriptorHeaps;
            eastl::map<uint32_t, DescriptorHeap_D3D12*> m_samplerHeaps;
            eastl::map<uint32_t, DescriptorHeap_D3D12*> m_cbvSrvUavHeaps;

            struct EmptyDescriptors_D3D12* m_pNullDescriptors;
            eastl::map<uint32_t, ID3D12CommandQueue**> m_commandQueues;
            eastl::map<uint32_t, uint32_t> m_commandQueueCounts;
            IDXGIFactory6* m_pDXGIFactory;
            IDXGIAdapter4* m_pDxActiveGPU;
            ID3D12Device* m_pDxDevice;

            // PSO Cache
            ID3D12PipelineLibrary* m_pPipelineLibrary;
            void* m_pPSOCacheData;
            
            uint64_t m_PadA;
            ID3D12Debug* m_pDxDebug;
            #if defined (_WINDOWS)
                ID3D12InfoQueue* m_pDxDebugValidation;
            #endif

            friend TextureImplType;
            friend BufferImplType;
            friend TextureView_D3D12_Impl;
        };
    }
}