#pragma once

#include "interface/render_device.h"
#include "backend/d3d12/rhi_d3d12.h"
#include "engine_impl_traits_d3d12.hpp"

namespace Cyber
{
    namespace RenderObject
    {
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
        public:
            // Device APIs
            virtual void create_device(IAdapter* adapter, const RenderDeviceCreateDesc& deviceDesc) override;
            virtual void free_device() override;
            // API Object APIs
            virtual RHISurface* surface_from_hwnd(HWND hwnd) override;
            virtual IFence* create_fence() override;
            virtual void wait_fences(IFence** fences, uint32_t fenceCount) override;
            virtual void free_fence(IFence* fence) override;
            virtual ERHIFenceStatus query_fence_status(IFence* fence) override;
            virtual ISwapChain* create_swap_chain(const SwapChainDesc& swapchainDesc) override;
            virtual void free_swap_chain(ISwapChain* swapchain) override;
            virtual void enum_adapters(IInstance* instance, IAdapter** adapters, uint32_t* adapterCount) override;
            virtual uint32_t acquire_next_image(ISwapChain* swapchain, const RHIAcquireNextDesc& acquireDesc) override;
            virtual IFrameBuffer* create_frame_buffer(const FrameBuffserDesc& frameBufferDesc) override;
            // Queue APIs
            virtual IQueue* get_queue(ERHIQueueType type, uint32_t index) override;
            virtual void submit_queue(IQueue* queue, const QueueSubmitDesc& submitDesc) override;
            virtual void present_queue(IQueue* queue, const QueuePresentDesc& presentDesc) override;
            virtual void wait_queue_idle(IQueue* queue) override;
            virtual ICommandPool* create_command_pool(IQueue* queue, const CommandPoolCreateDesc& commandPoolDesc) override;
            virtual void reset_command_pool(ICommandPool* pool) override;
            virtual void free_command_pool(ICommandPool* pool) override;
            virtual ICommandBuffer* create_command_buffer(ICommandPool* pool, const CommandBufferCreateDesc& commandBufferDesc) override;
            virtual void free_command_buffer(ICommandBuffer* commandBuffer) override;

            virtual void cmd_begin(ICommandBuffer* commandBuffer) override;
            virtual void cmd_end(ICommandBuffer* commandBuffer) override;
            virtual void cmd_resource_barrier(ICommandBuffer* cmd, const RHIResourceBarrierDesc& barrierDesc) override;

            virtual RHIRenderPassEncoder* cmd_begin_render_pass(ICommandBuffer* commandBuffer, const RenderPassDesc& beginRenderPassDesc) override;
            virtual void cmd_end_render_pass(ICommandBuffer* commandBuffer) override;
            virtual void render_encoder_bind_descriptor_set(RHIRenderPassEncoder* encoder, RHIDescriptorSet* descriptorSet) override;
            virtual void render_encoder_set_viewport(RHIRenderPassEncoder* encoder, float x, float y, float width, float height, float min_depth, float max_depth) override;
            virtual void render_encoder_set_scissor(RHIRenderPassEncoder* encoder, uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;
            virtual void render_encoder_bind_pipeline(RHIRenderPassEncoder* encoder, RHIRenderPipeline* pipeline) override;
            virtual void render_encoder_bind_vertex_buffer(RHIRenderPassEncoder* encoder, uint32_t buffer_count, IBuffer** buffers,const uint32_t* strides, const uint32_t* offsets) override;
            virtual void render_encoder_bind_index_buffer(RHIRenderPassEncoder* encoder, IBuffer* buffer, uint32_t index_stride, uint64_t offset) override;
            virtual void render_encoder_push_constants(RHIRenderPassEncoder* encoder, IRootSignature* rs, const char8_t* name, const void* data) override;
            virtual void render_encoder_draw(RHIRenderPassEncoder* encoder, uint32_t vertex_count, uint32_t first_vertex) override;
            virtual void render_encoder_draw_instanced(RHIRenderPassEncoder* encoder, uint32_t vertex_count, uint32_t first_vertex, uint32_t instance_count, uint32_t first_instance) override;
            virtual void render_encoder_draw_indexed(RHIRenderPassEncoder* encoder, uint32_t index_count, uint32_t first_index, uint32_t first_vertex) override;
            virtual void render_encoder_draw_indexed_instanced(RHIRenderPassEncoder*encoder, uint32_t index_count, uint32_t first_index, uint32_t instance_count, uint32_t first_instance, uint32_t first_vertex) override;
            
            virtual IRootSignature* create_root_signature(const RootSignatureCreateDesc& rootSigDesc) override;
            virtual void free_root_signature(IRootSignature* rootSignature) override;
            virtual RHIDescriptorSet* create_descriptor_set(const RHIDescriptorSetCreateDesc& dSetDesc) override;
            virtual void update_descriptor_set(RHIDescriptorSet* set, const RHIDescriptorData* updateDesc, uint32_t count) override;

            virtual IRenderPipeline* create_render_pipeline(const RenderPipelineCreateDesc& pipelineDesc) override;
            virtual void free_render_pipeline(RHIRenderPipeline* pipeline) override;
            virtual IInstance* create_instance(const InstanceCreateDesc& instanceDesc) override;
            virtual void free_instance(IInstance* instance) override;

            virtual ITextureView* create_texture_view(const RenderObject::TextureViewCreateDesc& viewDesc) override;
            virtual void free_texture_view(ITextureView* view) override;
            virtual ITexture* create_texture(const RenderObject::TextureCreateDesc& textureDesc) override;
            virtual IBuffer* create_buffer(const RenderObject::BufferCreateDesc& bufferDesc) override;
            virtual void free_buffer(IBuffer* buffer) override;
            virtual void map_buffer(IBuffer* buffer, const RHIBufferRange* range) override;
            virtual void unmap_buffer(IBuffer* buffer) override;

            virtual IShaderLibrary* create_shader_library(const struct ShaderLibraryCreateDesc& desc) override;
            virtual void free_shader_library(IShaderLibrary* shaderLibrary) override;

            // create view
            void create_constant_buffer_view(const D3D12_CONSTANT_BUFFER_VIEW_DESC* desc, D3D12_CPU_DESCRIPTOR_HANDLE destHandle);
            void create_shader_resource_view(ID3D12Resource* resource, const D3D12_SHADER_RESOURCE_VIEW_DESC* desc, D3D12_CPU_DESCRIPTOR_HANDLE destHandle);
            void create_unordered_access_view(ID3D12Resource* resource, ID3D12Resource* counterResource, const D3D12_UNORDERED_ACCESS_VIEW_DESC* desc, D3D12_CPU_DESCRIPTOR_HANDLE destHandle);
            void create_render_target_view(ID3D12Resource* resource, const D3D12_RENDER_TARGET_VIEW_DESC* desc, D3D12_CPU_DESCRIPTOR_HANDLE destHandle);

            ID3D12Device* GetD3D12Device() { return m_pDxDevice; }
            class D3D12MA::Allocator* GetD3D12ResourceAllocator() { return m_pResourceAllocator; }
            RHIDescriptorHeap_D3D12* GetCPUDescriptorHeaps(uint32_t heap_type) { return m_CPUDescriptorHeaps[heap_type]; }

            eastl::map<uint32_t, RHIDescriptorHeap_D3D12*>& GetSamplerHeaps() { return m_SamplerHeaps; }
            eastl::map<uint32_t, RHIDescriptorHeap_D3D12*>& GetCbvSrvUavHeaps() { return m_CbvSrvUavHeaps; }
            struct RHIEmptyDescriptors_D3D12* GetNullDescriptors() { return m_pNullDescriptors; }
            eastl::map<uint32_t, ID3D12CommandQueue**>& GetCommandQueues() { return m_ppCommandQueues; }
            eastl::map<uint32_t, uint32_t>& GetCommandQueueCounts() { return m_CommandQueueCounts; }
            IDXGIFactory6* GetDXGIFactory() { return m_pDXGIFactory; }
            IDXGIAdapter4* GetDXActiveGPU() { return m_pDxActiveGPU; }
            ID3D12PipelineLibrary* GetPipelineLibrary() { return m_pPipelineLibrary; }
            void* GetPSOCacheData() { return m_pPSOCacheData; }
            ID3D12Debug* GetDxDebug() { return m_pDxDebug; }

            void create_dma_allocallor(RenderObject::Adapter_D3D12_Impl* adapter);

        protected:
            HRESULT hook_CheckFeatureSupport(D3D12_FEATURE pFeature, void* pFeatureSupportData, UINT pFeatureSupportDataSize);
            HRESULT hook_CreateCommittedResource(const D3D12_HEAP_PROPERTIES* pHeapProperties, D3D12_HEAP_FLAGS HeapFlags, const D3D12_RESOURCE_DESC* pDesc, D3D12_RESOURCE_STATES InitialResourceState, const D3D12_CLEAR_VALUE* pOptimizedClearValue, REFIID riidResource, void **ppvResource);

            class D3D12MA::Allocator* m_pResourceAllocator;

            // API specific descriptor heap and memory allocator
            eastl::map<uint32_t, RHIDescriptorHeap_D3D12*> m_CPUDescriptorHeaps;
            eastl::map<uint32_t, RHIDescriptorHeap_D3D12*> m_SamplerHeaps;
            eastl::map<uint32_t, RHIDescriptorHeap_D3D12*> m_CbvSrvUavHeaps;
            struct RHIEmptyDescriptors_D3D12* m_pNullDescriptors;
            eastl::map<uint32_t, ID3D12CommandQueue**> m_ppCommandQueues;
            eastl::map<uint32_t, uint32_t> m_CommandQueueCounts;
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