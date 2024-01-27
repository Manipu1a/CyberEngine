#pragma once

#include "graphics/interface/render_pipeline.h"
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
        struct CYBER_GRAPHICS_API IRenderPipeline_D3D12 : public IRenderPipeline
        {
            
        };

        class CYBER_GRAPHICS_API RenderPipeline_D3D12_Impl : public RenderPipelineBase<EngineD3D12ImplTraits>
        {
        public:
            using TRenderPipelineBase = RenderPipelineBase<EngineD3D12ImplTraits>;
            using RenderDeviceImplType = EngineD3D12ImplTraits::RenderDeviceImplType;

            RenderPipeline_D3D12_Impl(class RenderDevice_D3D12_Impl* device) : TRenderPipelineBase(device) {}
        protected:
            ID3D12PipelineState* pDxPipelineState;
            ID3D12RootSignature* pDxRootSignature;
            D3D_PRIMITIVE_TOPOLOGY mPrimitiveTopologyType;
            friend class RenderObject::RenderDevice_D3D12_Impl;
        };

    }
}
