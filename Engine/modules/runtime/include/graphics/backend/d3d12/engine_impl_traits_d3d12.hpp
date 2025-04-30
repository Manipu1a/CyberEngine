#pragma once

namespace Cyber
{
    namespace RenderObject
    {
        class ITexture_D3D12;
        class ITextureView_D3D12;
        class IBuffer_D3D12;
        class IRenderDevice_D3D12;
        class ISwapChain_D3D12;
        class IFrameBuffer_D3D12;
        class ICommandBuffer_D3D12;
        class ICommandPool_D3D12;
        class IDeviceContext_D3D12;
        class IFence_D3D12;
        class IQueue_D3D12;
        class IInstance_D3D12;
        class IQueryPool_D3D12;
        class IRenderPipeline_D3D12;
        class IRootSignature_D3D12;
        class ISemaphore_D3D12;
        class IAdapter_D3D12;
        class IDescriptorSet_D3D12;
        class ISampler_D3D12;
        class IShaderReflection_D3D12;
        class IShaderLibrary_D3D12;
        class IShaderResource_D3D12;
        class IVertexInput_D3D12;
        class IRenderPass_D3D12;

        class Texture_D3D12_Impl;
        class TextureView_D3D12_Impl;
        class Buffer_D3D12_Impl;
        class RenderDevice_D3D12_Impl;
        class SwapChain_D3D12_Impl;
        class FrameBuffer_D3D12_Impl;
        class CommandBuffer_D3D12_Impl;
        class CommandPool_D3D12_Impl;
        class DeviceContext_D3D12_Impl;
        class Fence_D3D12_Impl;
        class Queue_D3D12_Impl;
        class Instance_D3D12_Impl;
        class QueryPool_D3D12_Impl;
        class RenderPipeline_D3D12_Impl;
        class RootSignature_D3D12_Impl;
        class Semaphore_D3D12_Impl;
        class Adapter_D3D12_Impl;
        class DescriptorSet_D3D12_Impl;
        class Sampler_D3D12_Impl;
        class ShaderReflection_D3D12_Impl;
        class ShaderLibrary_D3D12_Impl;
        class ShaderResource_D3D12_Impl;
        class VertexInput_D3D12_Impl;
        class RenderPass_D3D12_Impl;

        struct EngineD3D12ImplTraits
        {
            // Interface
            using TextureInterface = ITexture_D3D12;
            using TextureViewInterface = ITextureView_D3D12;
            using BufferInterface = IBuffer_D3D12;
            using RenderDeviceInterface = IRenderDevice_D3D12;
            using SwapChainInterface = ISwapChain_D3D12;
            using FrameBufferInterface = IFrameBuffer_D3D12;
            using CommandBufferInterface = ICommandBuffer_D3D12;
            using CommandPoolInterface = ICommandPool_D3D12;
            using DeviceContextInterface = IDeviceContext_D3D12;
            using FenceInterface = IFence_D3D12;
            using QueueInterface = IQueue_D3D12;
            using InstanceInterface = IInstance_D3D12;
            using QueryPoolInterface = IQueryPool_D3D12;
            using RenderPipelineInterface = IRenderPipeline_D3D12;
            using RootSignatureInterface = IRootSignature_D3D12;
            using SemaphoreInterface = ISemaphore_D3D12;
            using AdapterInterface = IAdapter_D3D12;
            using DescriptorSetInterface = IDescriptorSet_D3D12;
            using SamplerInterface = ISampler_D3D12;
            using ShaderReflectionInterface = IShaderReflection_D3D12;
            using ShaderLibraryInterface = IShaderLibrary_D3D12;
            using ShaderResourceInterface = IShaderResource_D3D12;
            using VertexInputInterface = IVertexInput_D3D12;
            using RenderPassInterface = IRenderPass_D3D12;

            // Impl
            using TextureImplType = Texture_D3D12_Impl;
            using TextureViewImplType = TextureView_D3D12_Impl;
            using BufferImplType = Buffer_D3D12_Impl;
            using RenderDeviceImplType = RenderDevice_D3D12_Impl;
            using SwapChainImplType = SwapChain_D3D12_Impl;
            using FrameBufferImplType = FrameBuffer_D3D12_Impl;
            using CommandBufferImplType = CommandBuffer_D3D12_Impl;
            using CommandPoolImplType = CommandPool_D3D12_Impl;
            using DeviceContextImplType = DeviceContext_D3D12_Impl;
            using FenceImplType = Fence_D3D12_Impl;
            using QueueImplType = Queue_D3D12_Impl;
            using InstanceImplType = Instance_D3D12_Impl;
            using QueryPoolImplType = QueryPool_D3D12_Impl;
            using RenderPipelineImplType = RenderPipeline_D3D12_Impl;
            using RootSignatureImplType = RootSignature_D3D12_Impl;
            using SemaphoreImplType = Semaphore_D3D12_Impl;
            using AdapterImplType = Adapter_D3D12_Impl;
            using DescriptorSetImplType = DescriptorSet_D3D12_Impl;
            using SamplerImplType = Sampler_D3D12_Impl;
            using ShaderReflectionImplType = ShaderReflection_D3D12_Impl;
            using ShaderLibraryImplType = ShaderLibrary_D3D12_Impl;
            using ShaderResourceImplType = ShaderResource_D3D12_Impl;
            using VertexInputImplType = VertexInput_D3D12_Impl;
            using RenderPassImplType = RenderPass_D3D12_Impl;
        };
    }
}