#pragma once

#include "engine_impl_traits_d3d12.hpp"
#include "interface/command_pool.h"

namespace Cyber
{
    namespace RenderObject
    {
        class RenderDevice_D3D12_Impl;
    }

    namespace RenderObject
    {
        // Buffer Interface
        struct CYBER_GRAPHICS_API ICommandPool_D3D12 : public ICommandPool
        {
            
        };

        class CYBER_GRAPHICS_API CommandPool_D3D12_Impl : public CommandPoolBase<EngineD3D12ImplTraits>
        {
        public:
            using TCommandPoolBase = CommandPoolBase<EngineD3D12ImplTraits>;
            using RenderDeviceImplType = EngineD3D12ImplTraits::RenderDeviceImplType;

            CommandPool_D3D12_Impl(class RenderDevice_D3D12_Impl* device, const CommandPoolCreateDesc& desc) : TCommandPoolBase(device, desc) {}

            ID3D12CommandAllocator* get_native_command_allocator() const { return m_pDxCmdAlloc; }
        protected:
            struct ID3D12CommandAllocator* m_pDxCmdAlloc;

            friend class RenderObject::RenderDevice_D3D12_Impl;
        };

    }
}