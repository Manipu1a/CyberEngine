#pragma once

#include "engine_impl_traits_d3d12.hpp"
#include "interface/vertex_input.h"

namespace Cyber
{
    namespace RenderObject
    {
        class RenderDevice_D3D12_Impl;
    }

    namespace RenderObject
    {
        // Buffer Interface
        struct CYBER_GRAPHICS_API IVertexInput_D3D12 : public IVertexInput
        {
            
        };

        class CYBER_GRAPHICS_API VertexInput_D3D12_Impl : public VertexInputBase<EngineD3D12ImplTraits>
        {
        public:
            using TVertexInputBase = VertexInputBase<EngineD3D12ImplTraits>;
            using RenderDeviceImplType = EngineD3D12ImplTraits::RenderDeviceImplType;

            VertexInput_D3D12_Impl(class RenderDevice_D3D12_Impl* device) : TVertexInputBase(device) {}

        protected:
            friend class RenderObject::RenderDevice_D3D12_Impl;
        };
    }
}