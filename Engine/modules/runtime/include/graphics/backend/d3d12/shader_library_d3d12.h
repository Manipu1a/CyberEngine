#pragma once

#include "engine_impl_traits_d3d12.hpp"
#include "interface/shader_library.h"
#include <d3d12shader.h>
#include <dxcapi.h>

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

            ShaderLibrary_D3D12_Impl(class RenderDevice_D3D12_Impl* device, const ShaderLibraryCreateDesc& desc) : TShaderLibraryBase(device, desc) {}

            virtual void free_reflection() override final;

            void Initialize_shader_reflection(const RenderObject::ShaderLibraryCreateDesc& desc);

            void collect_shader_reflection_data(ID3D12ShaderReflection* d3d12Reflection, SHADER_STAGE stage);

            void reflection_record_shader_resource( ID3D12ShaderReflection* d3d12Reflection, SHADER_STAGE stage, const D3D12_SHADER_DESC& shaderDesc);

            CYBER_FORCE_INLINE ID3DBlob* get_shader_blob() const { return m_pShaderBlob; }
        protected:
            ID3DBlob* m_pShaderBlob;
            struct IDxcResult* m_pShaderResult;

            friend class RenderObject::RenderDevice_D3D12_Impl;
        };
    }
}