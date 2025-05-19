#pragma once

#include "engine_impl_traits_d3d12.hpp"
#include "interface/root_signature.hpp"

namespace Cyber
{
    namespace RenderObject
    {
        class RenderDevice_D3D12_Impl;
    }

    namespace RenderObject
    {
        // Buffer Interface
        struct CYBER_GRAPHICS_API IRootSignature_D3D12 : public IRootSignature
        {
            
        };

        class CYBER_GRAPHICS_API RootSignature_D3D12_Impl : public RootSignatureBase<EngineD3D12ImplTraits>
        {
        public:
            using TRootSignatureBase = RootSignatureBase<EngineD3D12ImplTraits>;
            using RenderDeviceImplType = EngineD3D12ImplTraits::RenderDeviceImplType;

            RootSignature_D3D12_Impl(class RenderDevice_D3D12_Impl* device, const RootSignatureCreateDesc& desc) : TRootSignatureBase(device, desc) {}

        protected:
            ID3D12RootSignature* dxRootSignature;
            D3D12_ROOT_PARAMETER1 root_constant_parameter;
            uint32_t root_parameter_index;
            
            friend class DeviceContext_D3D12_Impl;
        };

    }
}