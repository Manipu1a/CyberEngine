#pragma once

#include "engine_impl_traits_d3d12.hpp"
#include "interface/root_signature.hpp"
#include "interface/shader_reflection.hpp"
#include "eastl/map.h"
#include "d3dx12_root_signature.h"

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
            static constexpr uint32_t MAX_ROOT_PARAMETERS = 32;

            eastl::map<SHADER_STAGE, RenderObject::ShaderRegisterCount> register_counts;
            ID3D12RootSignature* dxRootSignature;

            CD3DX12_ROOT_PARAMETER1 root_parameters[MAX_ROOT_PARAMETERS];
            CD3DX12_DESCRIPTOR_RANGE1 root_descriptor_ranges[MAX_ROOT_PARAMETERS];
            
            D3D12_ROOT_PARAMETER1 root_constant_parameter;
            uint32_t root_parameter_index;
            
            friend class DeviceContext_D3D12_Impl;
            friend class RenderDevice_D3D12_Impl;
        };

    }
}