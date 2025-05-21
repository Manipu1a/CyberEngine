#pragma once

#include "engine_impl_traits_d3d12.hpp"
#include "interface/graphics_types.h"
#include "interface/command_buffer.h"
#include "platform/memory.h"

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

            CommandBuffer_D3D12_Impl(class RenderDevice_D3D12_Impl* device, const CommandBufferCreateDesc& desc) : TCommandBufferBase(device, desc) 
            {
                m_pDxCmdList = nullptr;
                //m_pBoundHeaps[0] = nullptr;
                //m_pBoundHeaps[1] = nullptr;
                //m_boundHeapStartHandles[0].ptr = 0;
                //m_boundHeapStartHandles[1].ptr = 0;
               // m_pBoundRootSignature = nullptr;
               // m_type = 0;
                //m_nodeIndex = 0;
                //m_pCmdPool = nullptr;
            }

            virtual ~CommandBuffer_D3D12_Impl() = default;

            ID3D12GraphicsCommandList** get_dx_cmd_list_addr() { return &m_pDxCmdList; }
            ID3D12GraphicsCommandList* get_dx_cmd_list() const { return m_pDxCmdList; }
            void set_dx_cmd_list(ID3D12GraphicsCommandList* cmdList) { m_pDxCmdList = cmdList; }
            /*
            struct DescriptorHeap_D3D12* get_bound_heap(uint32_t index) const { return m_pBoundHeaps[index]; }
            void set_bound_heap(uint32_t index, struct DescriptorHeap_D3D12* heap) { m_pBoundHeaps[index] = heap; }

            D3D12_GPU_DESCRIPTOR_HANDLE get_bound_heap_start_handle(uint32_t index) const { return m_boundHeapStartHandles[index]; }
            void set_bound_heap_start_handle(uint32_t index, D3D12_GPU_DESCRIPTOR_HANDLE handle) { m_boundHeapStartHandles[index] = handle; }

            const ID3D12RootSignature* get_bound_root_signature() const { return m_pBoundRootSignature; }
            void set_bound_root_signature(const ID3D12RootSignature* rootSignature) { m_pBoundRootSignature = rootSignature; }

            uint32_t get_type() const { return m_type; }
            void set_type(uint32_t type) { m_type = type; }

            uint32_t get_node_index() const { return m_nodeIndex; }
            void set_node_index(uint32_t nodeIndex) { m_nodeIndex = nodeIndex; }

            ICommandPool* get_cmd_pool() const { return m_pCmdPool; }
            void set_cmd_pool(ICommandPool* cmdPool) { m_pCmdPool = cmdPool; }
            */
            virtual void free() override final
            {
                SAFE_RELEASE(m_pDxCmdList);
                TCommandBufferBase::free();
            }
        protected:
            ID3D12GraphicsCommandList* m_pDxCmdList;

            friend class RenderObject::RenderDevice_D3D12_Impl;
            friend class RenderObject::DeviceContext_D3D12_Impl;
        };

    }
}