#pragma once

#include "interface/render_device.h"
#include "backend/d3d12/rhi_d3d12.h"

namespace Cyber
{
    namespace RenderObject
    {
        class Texture_D3D12;
        class Buffer_D3D12;
        class Texture_View_D3D12;

        class CERenderDevice_D3D12 : public CERenderDevice
        {

        public:
            CERenderDevice_D3D12(RHIAdapter* adapter, const RenderDeviceCreateDesc& deviceDesc);
            virtual ~CERenderDevice_D3D12();
        public:

            // Device APIs
            virtual void create_device(RHIAdapter* adapter, const RenderDeviceCreateDesc& deviceDesc) override;
            virtual void free_device() override;
            // API Object APIs
            virtual RHISurface* surface_from_hwnd(HWND hwnd) override;
            virtual RHIFence* create_fence() override;
            virtual void wait_fences(RHIFence** fences, uint32_t fenceCount) override;
            virtual void free_fence(RHIFence* fence) override;
            virtual ERHIFenceStatus query_fence_status(RHIFence* fence) override;
            virtual RHISwapChain* create_swap_chain(const RHISwapChainCreateDesc& swapchainDesc) override;
            virtual void free_swap_chain(RHISwapChain* swapchain) override;
            virtual void enum_adapters(RHIInstance* instance, RHIAdapter** adapters, uint32_t* adapterCount) override;
            virtual uint32_t acquire_next_image(RHISwapChain* swapchain, const RHIAcquireNextDesc& acquireDesc) override;
            virtual CEFrameBuffer* create_frame_buffer(const FrameBuffserDesc& frameBufferDesc) override;
            // Queue APIs
            virtual RHIQueue* get_queue(ERHIQueueType type, uint32_t index) override;
            virtual void submit_queue(RHIQueue* queue, const RHIQueueSubmitDesc& submitDesc) override;
            virtual void present_queue(RHIQueue* queue, const RHIQueuePresentDesc& presentDesc) override;
            virtual void wait_queue_idle(RHIQueue* queue) override;
            virtual RHICommandPool* create_command_pool(RHIQueue* queue, const CommandPoolCreateDesc& commandPoolDesc) override;
            virtual void reset_command_pool(RHICommandPool* pool) override;
            virtual void free_command_pool(RHICommandPool* pool) override;
            virtual RHICommandBuffer* create_command_buffer(RHICommandPool* pool, const CommandBufferCreateDesc& commandBufferDesc) override;
            virtual void free_command_buffer(RHICommandBuffer* commandBuffer) override;

            virtual void cmd_begin(RHICommandBuffer* commandBuffer) override;
            virtual void cmd_end(RHICommandBuffer* commandBuffer) override;
            virtual void cmd_resource_barrier(RHICommandBuffer* cmd, const RHIResourceBarrierDesc& barrierDesc) override;

            virtual RHIRenderPassEncoder* cmd_begin_render_pass(RHICommandBuffer* commandBuffer, const RHIRenderPassDesc& beginRenderPassDesc) override;
            virtual void cmd_end_render_pass(RHICommandBuffer* commandBuffer) override;
            virtual void render_encoder_bind_descriptor_set(RHIRenderPassEncoder* encoder, RHIDescriptorSet* descriptorSet) override;
            virtual void render_encoder_set_viewport(RHIRenderPassEncoder* encoder, float x, float y, float width, float height, float min_depth, float max_depth) override;
            virtual void render_encoder_set_scissor(RHIRenderPassEncoder* encoder, uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;
            virtual void render_encoder_bind_pipeline(RHIRenderPassEncoder* encoder, RHIRenderPipeline* pipeline) override;
            virtual void render_encoder_bind_vertex_buffer(RHIRenderPassEncoder* encoder, uint32_t buffer_count, RenderObject::Buffer** buffers,const uint32_t* strides, const uint32_t* offsets) override;
            virtual void render_encoder_bind_index_buffer(RHIRenderPassEncoder* encoder, RenderObject::Buffer* buffer, uint32_t index_stride, uint64_t offset) override;
            virtual void render_encoder_push_constants(RHIRenderPassEncoder* encoder, RHIRootSignature* rs, const char8_t* name, const void* data) override;
            virtual void render_encoder_draw(RHIRenderPassEncoder* encoder, uint32_t vertex_count, uint32_t first_vertex) override;
            virtual void render_encoder_draw_instanced(RHIRenderPassEncoder* encoder, uint32_t vertex_count, uint32_t first_vertex, uint32_t instance_count, uint32_t first_instance) override;
            virtual void render_encoder_draw_indexed(RHIRenderPassEncoder* encoder, uint32_t index_count, uint32_t first_index, uint32_t first_vertex) override;
            virtual void render_encoder_draw_indexed_instanced(RHIRenderPassEncoder*encoder, uint32_t index_count, uint32_t first_index, uint32_t instance_count, uint32_t first_instance, uint32_t first_vertex) override;
            
            virtual RHIRootSignature* create_root_signature(const RHIRootSignatureCreateDesc& rootSigDesc) override;
            virtual void free_root_signature(RHIRootSignature* rootSignature) override;
            virtual RHIDescriptorSet* create_descriptor_set(const RHIDescriptorSetCreateDesc& dSetDesc) override;
            virtual void update_descriptor_set(RHIDescriptorSet* set, const RHIDescriptorData* updateDesc, uint32_t count) override;

            virtual RHIRenderPipeline* create_render_pipeline(const RHIRenderPipelineCreateDesc& pipelineDesc) override;
            virtual void free_render_pipeline(RHIRenderPipeline* pipeline) override;
            virtual RHIInstance* create_instance(const RHIInstanceCreateDesc& instanceDesc) override;
            virtual void free_instance(RHIInstance* instance) override;

            virtual RenderObject::Texture_View* create_texture_view(const RenderObject::TextureViewCreateDesc& viewDesc) override;
            virtual void free_texture_view(RenderObject::Texture_View* view) override;
            virtual RenderObject::Texture* create_texture(const RenderObject::TextureCreateDesc& textureDesc) override;
            virtual RenderObject::Buffer* create_buffer(const RenderObject::BufferCreateDesc& bufferDesc) override;
            virtual void free_buffer(RenderObject::Buffer* buffer) override;
            virtual void map_buffer(RenderObject::Buffer* buffer, const RHIBufferRange* range) override;
            virtual void unmap_buffer(RenderObject::Buffer* buffer) override;

            virtual RHIShaderLibrary* create_shader_library(const struct RHIShaderLibraryCreateDesc& desc) override;
            virtual void free_shader_library(RHIShaderLibrary* shaderLibrary) override;

            ID3D12Device* GetD3D12Device() { return pDxDevice; }
            class D3D12MA::Allocator* GetD3D12ResourceAllocator() { return pResourceAllocator; }
            RHIDescriptorHeap_D3D12* GetCPUDescriptorHeaps(uint32_t heap_type) { return mCPUDescriptorHeaps[heap_type]; }

            eastl::map<uint32_t, RHIDescriptorHeap_D3D12*>& GetSamplerHeaps() { return mSamplerHeaps; }
            eastl::map<uint32_t, RHIDescriptorHeap_D3D12*>& GetCbvSrvUavHeaps() { return mCbvSrvUavHeaps; }
            struct RHIEmptyDescriptors_D3D12* GetNullDescriptors() { return pNullDescriptors; }
            eastl::map<uint32_t, ID3D12CommandQueue**>& GetCommandQueues() { return ppCommandQueues; }
            eastl::map<uint32_t, uint32_t>& GetCommandQueueCounts() { return pCommandQueueCounts; }
            IDXGIFactory6* GetDXGIFactory() { return pDXGIFactory; }
            IDXGIAdapter4* GetDXActiveGPU() { return pDxActiveGPU; }
            ID3D12PipelineLibrary* GetPipelineLibrary() { return pPipelineLibrary; }
            void* GetPSOCacheData() { return pPSOCacheData; }
            ID3D12Debug* GetDxDebug() { return pDxDebug; }

        protected:
            HRESULT hook_CheckFeatureSupport(D3D12_FEATURE pFeature, void* pFeatureSupportData, UINT pFeatureSupportDataSize);
            HRESULT hook_CreateCommittedResource(const D3D12_HEAP_PROPERTIES* pHeapProperties, D3D12_HEAP_FLAGS HeapFlags, const D3D12_RESOURCE_DESC* pDesc, D3D12_RESOURCE_STATES InitialResourceState, const D3D12_CLEAR_VALUE* pOptimizedClearValue, REFIID riidResource, void **ppvResource);

            class D3D12MA::Allocator* pResourceAllocator;

            // API specific descriptor heap and memory allocator
            eastl::map<uint32_t, RHIDescriptorHeap_D3D12*> mCPUDescriptorHeaps;
            eastl::map<uint32_t, RHIDescriptorHeap_D3D12*> mSamplerHeaps;
            eastl::map<uint32_t, RHIDescriptorHeap_D3D12*> mCbvSrvUavHeaps;
            struct RHIEmptyDescriptors_D3D12* pNullDescriptors;
            eastl::map<uint32_t, ID3D12CommandQueue**> ppCommandQueues;
            eastl::map<uint32_t, uint32_t> pCommandQueueCounts;
            IDXGIFactory6* pDXGIFactory;
            IDXGIAdapter4* pDxActiveGPU;
            ID3D12Device* pDxDevice;

            // PSO Cache
            ID3D12PipelineLibrary* pPipelineLibrary;
            void* pPSOCacheData;
            
            uint64_t mPadA;
            ID3D12Debug* pDxDebug;
            #if defined (_WINDOWS)
                ID3D12InfoQueue* pDxDebugValidation;
            #endif

            friend class RenderObject::Texture_D3D12;
            friend class RenderObject::Buffer_D3D12;
            friend class RenderObject::Texture_View_D3D12;
        };
    }
}