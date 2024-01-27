#pragma once

#include "engine_impl_traits_d3d12.hpp"
#include "interface/rhi.h"
#include "interface/command_buffer.h"

namespace Cyber
{
    namespace RenderObject
    {
        class RenderDevice_D3D12_Impl;
    }

    namespace RenderObject
    {
        // Buffer Interface
        struct CYBER_GRAPHICS_API ICommandBuffer_D3D12 : public ICommandBuffer
        {
            
        };

        class CYBER_GRAPHICS_API CommandBuffer_D3D12_Impl : public CommandBufferBase<EngineD3D12ImplTraits>
        {
        public:
            using TCommandBufferBase = CommandBufferBase<EngineD3D12ImplTraits>;
            using RenderDeviceImplType = EngineD3D12ImplTraits::RenderDeviceImplType;

            CommandBuffer_D3D12_Impl(class RenderDevice_D3D12_Impl* device) : TCommandBufferBase(device) {}

        protected:
            ID3D12GraphicsCommandList* pDxCmdList;
            // Cached in beginCmd to avoid fetching them during rendering
            struct RHIDescriptorHeap_D3D12* pBoundHeaps[2];
            D3D12_GPU_DESCRIPTOR_HANDLE mBoundHeapStartHandles[2];
            // Command buffer state
            const ID3D12RootSignature* pBoundRootSignature;
            uint32_t mType;
            uint32_t mNodeIndex;
            ICommandPool* pCmdPool;
            D3D12_RENDER_PASS_ENDING_ACCESS_RESOLVE_SUBRESOURCE_PARAMETERS mSubResolveResource[RHI_MAX_MRT_COUNT];
            friend class RenderObject::RenderDevice_D3D12_Impl;
        };

    }
}