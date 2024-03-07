#include "d3d12_utils.h"
//#include "platform/configure.h"
//#include <winnt.h>
#include <d3d12.h>
#include <dxcapi.h>
#include <d3d12shader.h>
#include <d3dcompiler.h>
#include "platform/memory.h"
//#include "cyber_runtime.config.h"
//#include "../../common/common_utils.h"
#include "graphics/backend/d3d12/shader_library_d3d12.h"
#include "graphics/backend/d3d12/adapter_d3d12.h"
#include "graphics/backend/d3d12/render_device_d3d12.h"
#include "graphics/backend/d3d12/queue_d3d12.h"
#include "graphics/backend/d3d12/instance_d3d12.h"
#include "graphics/backend/d3d12/shader_reflection_d3d12.h"
#include "graphics/backend/d3d12/shader_resource_d3d12.h"
#include "graphics/backend/d3d12/vertex_input_d3d12.h"
#include "graphics/interface/vertex_input.h"

namespace Cyber
{

    void D3D12Util_InitializeEnvironment(RenderObject::Instance_D3D12_Impl* pInst)
    {
        
    }

    void D3D12Util_DeInitializeEnvironment(RenderObject::Instance_D3D12_Impl* pInst)
    {
        
    }

    D3D12_BLEND_DESC D3D12Util_TranslateBlendState(const BlendStateCreateDesc* pDesc)
    {
        int blendDescIndex = 0;
        D3D12_BLEND_DESC ret = {};
        ret.AlphaToCoverageEnable = (BOOL)pDesc->alpha_to_coverage;
        ret.IndependentBlendEnable = TRUE;
        for(int i = 0; i < GRAPHICS_MAX_MRT_COUNT;++i)
        {
            BOOL blendEnable = (gDx12BlendConstantTranslator[pDesc->src_factors[blendDescIndex]] != D3D12_BLEND_ONE) || (gDx12BlendConstantTranslator[pDesc->dst_factors[blendDescIndex]] != D3D12_BLEND_ZERO) ||
                                (gDx12BlendConstantTranslator[pDesc->src_alpha_factors[blendDescIndex]] != D3D12_BLEND_ONE) || (gDx12BlendConstantTranslator[pDesc->dst_alpha_factors[blendDescIndex]] != D3D12_BLEND_ZERO);

            ret.RenderTarget[i].BlendEnable = blendEnable;
            ret.RenderTarget[i].RenderTargetWriteMask = (UINT8)pDesc->masks[blendDescIndex];
            ret.RenderTarget[i].BlendOp = gDx12BlendOpTranlator[pDesc->blend_modes[blendDescIndex]];
            ret.RenderTarget[i].SrcBlend = gDx12BlendConstantTranslator[pDesc->src_factors[blendDescIndex]];
            ret.RenderTarget[i].DestBlend = gDx12BlendConstantTranslator[pDesc->dst_factors[blendDescIndex]];
            ret.RenderTarget[i].BlendOpAlpha = gDx12BlendOpTranlator[pDesc->blend_alpha_modes[blendDescIndex]];
            ret.RenderTarget[i].SrcBlendAlpha = gDx12BlendConstantTranslator[pDesc->src_alpha_factors[blendDescIndex]];
            ret.RenderTarget[i].DestBlendAlpha = gDx12BlendConstantTranslator[pDesc->dst_alpha_factors[blendDescIndex]];

            if(pDesc->independent_blend)
            {
                ++blendDescIndex;
            }
        }
        return ret;
    }
    
    D3D12_RASTERIZER_DESC D3D12Util_TranslateRasterizerState(const RasterizerStateCreateDesc* pDesc)
    {
        cyber_check(pDesc->fill_mode < FILL_MODE_COUNT);
        cyber_check(pDesc->cull_mode < CULL_MODE_COUNT);
        cyber_check(pDesc->front_face == FRONT_FACE_COUNTER_CLOCKWISE || pDesc->front_face == FRONT_FACE_CLOCKWISE);
        D3D12_RASTERIZER_DESC ret = {};
        ret.FillMode = gDx12FillModeTranslator[pDesc->fill_mode];
        ret.CullMode = gDx12CullModeTranslator[pDesc->cull_mode];
        ret.FrontCounterClockwise = (pDesc->front_face == FRONT_FACE_COUNTER_CLOCKWISE);
        ret.DepthBias = pDesc->depth_bias;
        ret.DepthBiasClamp = 0.0f;
        ret.SlopeScaledDepthBias = pDesc->slope_scaled_depth_bias;
        ret.DepthClipEnable = pDesc->enable_depth_clip;
        ret.MultisampleEnable = pDesc->enable_multisample ? TRUE : FALSE;
        ret.AntialiasedLineEnable = false;
        ret.ForcedSampleCount = 0;
        ret.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
        return ret;
    }

    D3D12_DEPTH_STENCIL_DESC D3D12Util_TranslateDepthStencilState(const DepthStateCreateDesc* pDesc)
    {
        cyber_check(pDesc->depth_func < CMP_COUNT);
        cyber_check(pDesc->stencil_front_func < CMP_COUNT);
        cyber_check(pDesc->stencil_front_fail_op < STENCIL_OP_COUNT);
        cyber_check(pDesc->stencil_front_pass_op < STENCIL_OP_COUNT);
        cyber_check(pDesc->stencil_front_depth_fail_op < STENCIL_OP_COUNT);
        cyber_check(pDesc->stencil_back_fail_op < STENCIL_OP_COUNT);
        cyber_check(pDesc->stencil_back_pass_op < STENCIL_OP_COUNT);
        cyber_check(pDesc->stencil_back_depth_fail_op < STENCIL_OP_COUNT);

        D3D12_DEPTH_STENCIL_DESC ret = {};
        ret.DepthEnable = pDesc->depth_test ? TRUE : FALSE;
        ret.DepthWriteMask = pDesc->depth_write ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
        ret.DepthFunc = gDx12ComparisonFuncTranslator[pDesc->depth_func];
        ret.StencilEnable = pDesc->stencil_test ? TRUE : FALSE;
        ret.StencilReadMask = pDesc->stencil_read_mask;
        ret.StencilWriteMask = pDesc->stencil_write_mask;
        ret.FrontFace.StencilFunc = gDx12ComparisonFuncTranslator[pDesc->stencil_front_func];
        ret.FrontFace.StencilFailOp = gDx12StencilOpTranslator[pDesc->stencil_front_fail_op];
        ret.FrontFace.StencilDepthFailOp = gDx12StencilOpTranslator[pDesc->stencil_front_depth_fail_op];
        ret.FrontFace.StencilPassOp = gDx12StencilOpTranslator[pDesc->stencil_front_pass_op];
        ret.BackFace.StencilFunc = gDx12ComparisonFuncTranslator[pDesc->stencil_back_func];
        ret.BackFace.StencilFailOp = gDx12StencilOpTranslator[pDesc->stencil_back_fail_op];
        ret.BackFace.StencilDepthFailOp = gDx12StencilOpTranslator[pDesc->stencil_back_depth_fail_op];
        ret.BackFace.StencilPassOp = gDx12StencilOpTranslator[pDesc->stencil_back_pass_op];
        return ret;
    }

    D3D12_PRIMITIVE_TOPOLOGY_TYPE D3D12Util_TranslatePrimitiveTopologyType(PRIMITIVE_TOPOLOGY topology)
    {
        switch(topology)
        {
            case PRIMITIVE_TOPOLOGY::PRIM_TOPO_POINT_LIST: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
            case PRIMITIVE_TOPOLOGY::PRIM_TOPO_LINE_LIST:
            case PRIMITIVE_TOPOLOGY::PRIM_TOPO_LINE_STRIP: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
            case PRIMITIVE_TOPOLOGY::PRIM_TOPO_TRIANGLE_LIST:
            case PRIMITIVE_TOPOLOGY::PRIM_TOPO_TRIANGLE_STRIP: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
            case PRIMITIVE_TOPOLOGY::PRIM_TOPO_PATCH_LIST: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
        }
        cyber_check(false);
        return D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
    }
}