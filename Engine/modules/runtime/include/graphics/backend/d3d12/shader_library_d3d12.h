#pragma once

#include "engine_impl_traits_d3d12.hpp"
#include "interface/shader_library.h"

namespace Cyber
{
    namespace RenderObject
    {
        class RenderDevice_D3D12_Impl;
    }

    namespace RenderObject
    {
        // Buffer Interface
        struct CYBER_GRAPHICS_API IShaderLibrary_D3D12 : public IShaderLibrary
        {
            
        };

        class CYBER_GRAPHICS_API ShaderLibrary_D3D12_Impl : public ShaderLibraryBase<EngineD3D12ImplTraits>
        {
        public:
            using TShaderLibraryBase = ShaderLibraryBase<EngineD3D12ImplTraits>;
            using RenderDeviceImplType = EngineD3D12ImplTraits::RenderDeviceImplType;

            ShaderLibrary_D3D12_Impl(class RenderDevice_D3D12_Impl* device) : TShaderLibraryBase(device) {}

        protected:
            ID3DBlob* shader_blob;
            //struct IDxcBlobEncoding* shader_dxc_blob;
            struct IDxcResult* shader_result;

            friend class RenderObject::RenderDevice_D3D12_Impl;
        };

    }
}