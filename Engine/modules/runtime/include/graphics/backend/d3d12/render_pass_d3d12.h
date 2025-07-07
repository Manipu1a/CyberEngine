#pragma once

#include "graphics/interface/render_pass.h"
#include "engine_impl_traits_d3d12.hpp"

namespace Cyber
{
    namespace RenderObject
    {
        class RenderDevice_D3D12_Impl;
    }

    namespace RenderObject
    {
        // Buffer Interface
        struct CYBER_GRAPHICS_API IRenderPass_D3D12 : public IRenderPass
        {
            
        };

        class CYBER_GRAPHICS_API RenderPass_D3D12_Impl : public RenderPassBase<EngineD3D12ImplTraits>
        {
        public:
            using TRenderPassBase = RenderPassBase<EngineD3D12ImplTraits>;
            using RenderDeviceImplType = EngineD3D12ImplTraits::RenderDeviceImplType;

            RenderPass_D3D12_Impl(RenderDeviceImplType* device, const RenderPassDesc& desc) : TRenderPassBase(device, desc) {}
        protected:
            //ID3D12PipelineState* pDxPipelineState;
            //ID3D12RootSignature* pDxRootSignature;
            
            D3D_PRIMITIVE_TOPOLOGY mPrimitiveTopologyType;
            friend class RenderObject::RenderDevice_D3D12_Impl;
        };

    }
}
