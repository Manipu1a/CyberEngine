#pragma once

#include "engine_impl_traits_d3d12.hpp"
#include "interface/shader_resource.hpp"

namespace Cyber
{
    namespace RenderObject
    {
        class RenderDevice_D3D12_Impl;
    }

    namespace RenderObject
    {
        // Buffer Interface
        struct CYBER_GRAPHICS_API IShaderResource_D3D12 : public IShaderResource
        {
            
        };

        class CYBER_GRAPHICS_API ShaderResource_D3D12_Impl : public ShaderResourceBase<EngineD3D12ImplTraits>
        {
        public:
            using TShaderResourceBase = ShaderResourceBase<EngineD3D12ImplTraits>;
            using RenderDeviceImplType = EngineD3D12ImplTraits::RenderDeviceImplType;

            ShaderResource_D3D12_Impl(class RenderDevice_D3D12_Impl* device) : TShaderResourceBase(device) {}

        protected:
            friend class RenderObject::RenderDevice_D3D12_Impl;
        };
    }
}