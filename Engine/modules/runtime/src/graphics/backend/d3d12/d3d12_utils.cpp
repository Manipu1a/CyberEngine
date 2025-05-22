#include "graphics/backend/d3d12/d3d12_utils.h"
//#include "platform/configure.h"
#include <d3d12.h>
#include <d3d12shader.h>
#include <d3dcompiler.h>
#include "platform/memory.h"
#include "EASTL/string.h"
//#include "cyber_runtime.config.h"
//#include "../../common/common_utils.h"
#include "graphics/backend/d3d12/shader_library_d3d12.h"
#include "graphics/backend/d3d12/adapter_d3d12.h"
#include "graphics/backend/d3d12/render_device_d3d12.h"
#include "graphics/backend/d3d12/command_queue_d3d12.h"
#include "graphics/backend/d3d12/instance_d3d12.h"
#include "graphics/backend/d3d12/shader_reflection_d3d12.h"
#include "graphics/backend/d3d12/shader_resource_d3d12.h"
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

    D3D12_FILTER D3D12Util_TranslateFilter(FILTER_TYPE minFilter, FILTER_TYPE magFilter, FILTER_TYPE mipfilter)
    {
        switch (minFilter)
        {
            case FILTER_TYPE_UNKNOWN:
                CB_CORE_ERROR("Unknown filter type");
                break;
            case FILTER_TYPE::FILTER_TYPE_POINT:
            {
                if(magFilter == FILTER_TYPE_POINT)
                {
                    if(mipfilter == FILTER_TYPE_POINT)
                    {
                        return D3D12_FILTER_MIN_MAG_MIP_POINT;
                    }
                    else if(mipfilter == FILTER_TYPE_LINEAR)
                    {
                        return D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR;
                    }
                }
                else if(magFilter == FILTER_TYPE_LINEAR)
                {
                    if(mipfilter == FILTER_TYPE_POINT)
                    {
                        return D3D12_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
                    }
                    else if(mipfilter == FILTER_TYPE_LINEAR)
                    {
                        return D3D12_FILTER_MIN_POINT_MAG_MIP_LINEAR;
                    }
                }
                break;
            }
            case FILTER_TYPE::FILTER_TYPE_LINEAR:
            {
                if(magFilter == FILTER_TYPE_POINT)
                {
                    if(mipfilter == FILTER_TYPE_POINT)
                    {
                        return D3D12_FILTER_MIN_LINEAR_MAG_MIP_POINT;
                    }
                    else if(mipfilter == FILTER_TYPE_LINEAR)
                    {
                        return D3D12_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
                    }
                }
                else if(magFilter == FILTER_TYPE_LINEAR)
                {
                    if(mipfilter == FILTER_TYPE_POINT)
                    {
                        return D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
                    }
                    else if(mipfilter == FILTER_TYPE_LINEAR)
                    {
                        return D3D12_FILTER_MIN_MAG_MIP_LINEAR;
                    }
                }
                break;
            }

            case FILTER_TYPE::FILTER_TYPE_ANISOTROPIC:
            {
                cyber_assert(magFilter == FILTER_TYPE_ANISOTROPIC && mipfilter == FILTER_TYPE_ANISOTROPIC, "For anisotropic filtering, both mag and min filters must be anisotropic.");
                return D3D12_FILTER_ANISOTROPIC;
                break;
            }

            case FILTER_TYPE::FILTER_TYPE_COMPARISON_POINT:
            {
                if(magFilter == FILTER_TYPE_COMPARISON_POINT)
                {
                    if(mipfilter == FILTER_TYPE_COMPARISON_POINT)
                    {
                        return D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
                    }
                    else if(mipfilter == FILTER_TYPE_COMPARISON_LINEAR)
                    {
                        return D3D12_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR;
                    }
                }
                else if(magFilter == FILTER_TYPE_COMPARISON_LINEAR)
                {
                    if(mipfilter == FILTER_TYPE_COMPARISON_POINT)
                    {
                        return D3D12_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT;
                    }
                    else if(mipfilter == FILTER_TYPE_COMPARISON_LINEAR)
                    {
                        return D3D12_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR;
                    }
                }
                break;
            }

            case FILTER_TYPE::FILTER_TYPE_COMPARISON_LINEAR:
            {
                if(magFilter == FILTER_TYPE_COMPARISON_POINT)
                {
                    if(mipfilter == FILTER_TYPE_COMPARISON_POINT)
                    {
                        return D3D12_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT;
                    }
                    else if(mipfilter == FILTER_TYPE_COMPARISON_LINEAR)
                    {
                        return D3D12_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
                    }
                }
                else if(magFilter == FILTER_TYPE_COMPARISON_LINEAR)
                {
                    if(mipfilter == FILTER_TYPE_COMPARISON_POINT)
                    {
                        return D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
                    }
                    else if(mipfilter == FILTER_TYPE_COMPARISON_LINEAR)
                    {
                        return D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
                    }
                }
                break;
            }

            case FILTER_TYPE::FILTER_TYPE_COMPARISON_ANISOTROPIC:
            {
                cyber_assert(magFilter == FILTER_TYPE_COMPARISON_ANISOTROPIC && mipfilter == FILTER_TYPE_COMPARISON_ANISOTROPIC, "For anisotropic filtering, both mag and min filters must be anisotropic.");
                return D3D12_FILTER_COMPARISON_ANISOTROPIC;
                break;
            }

            // Minimum filters
            case FILTER_TYPE::FILTER_TYPE_MINIMUM_POINT:
            {
                if(magFilter == FILTER_TYPE_MINIMUM_POINT)
                {
                    if(mipfilter == FILTER_TYPE_MINIMUM_POINT)
                    {
                        return D3D12_FILTER_MINIMUM_MIN_MAG_MIP_POINT;
                    }
                    else if(mipfilter == FILTER_TYPE_MINIMUM_LINEAR)
                    {
                        return D3D12_FILTER_MINIMUM_MIN_MAG_POINT_MIP_LINEAR;
                    }
                }
                else if(magFilter == FILTER_TYPE_MINIMUM_LINEAR)
                {
                    if(mipfilter == FILTER_TYPE_MINIMUM_POINT)
                    {
                        return D3D12_FILTER_MINIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT;
                    }
                    else if(mipfilter == FILTER_TYPE_MINIMUM_LINEAR)
                    {
                        return D3D12_FILTER_MINIMUM_MIN_POINT_MAG_MIP_LINEAR;
                    }
                }
                break;
            }

            case FILTER_TYPE::FILTER_TYPE_MINIMUM_LINEAR:
            {
                if(magFilter == FILTER_TYPE_MINIMUM_POINT)
                {
                    if(mipfilter == FILTER_TYPE_MINIMUM_POINT)
                    {
                        return D3D12_FILTER_MINIMUM_MIN_LINEAR_MAG_MIP_POINT;
                    }
                    else if(mipfilter == FILTER_TYPE_MINIMUM_LINEAR)
                    {
                        return D3D12_FILTER_MINIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
                    }
                }
                else if(magFilter == FILTER_TYPE_MINIMUM_LINEAR)
                {
                    if(mipfilter == FILTER_TYPE_MINIMUM_POINT)
                    {
                        return D3D12_FILTER_MINIMUM_MIN_MAG_LINEAR_MIP_POINT;
                    }
                    else if(mipfilter == FILTER_TYPE_MINIMUM_LINEAR)
                    {
                        return D3D12_FILTER_MINIMUM_MIN_MAG_MIP_LINEAR;
                    }
                }
                break;
            }

            case FILTER_TYPE::FILTER_TYPE_MINIMUM_ANISOTROPIC:
            {
                cyber_assert(magFilter == FILTER_TYPE_MINIMUM_ANISOTROPIC && mipfilter == FILTER_TYPE_MINIMUM_ANISOTROPIC, "For anisotropic filtering, both mag and min filters must be anisotropic.");
                return D3D12_FILTER_MINIMUM_ANISOTROPIC;
                break;
            }

            // Maximum filters
            case FILTER_TYPE::FILTER_TYPE_MAXIMUM_POINT:
            {
                if(magFilter == FILTER_TYPE_MAXIMUM_POINT)
                {
                    if(mipfilter == FILTER_TYPE_MAXIMUM_POINT)
                    {
                        return D3D12_FILTER_MAXIMUM_MIN_MAG_MIP_POINT;
                    }
                    else if(mipfilter == FILTER_TYPE_MAXIMUM_LINEAR)
                    {
                        return D3D12_FILTER_MAXIMUM_MIN_MAG_POINT_MIP_LINEAR;
                    }
                }
                else if(magFilter == FILTER_TYPE_MAXIMUM_LINEAR)
                {
                    if(mipfilter == FILTER_TYPE_MAXIMUM_POINT)
                    {
                        return D3D12_FILTER_MAXIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT;
                    }
                    else if(mipfilter == FILTER_TYPE_MAXIMUM_LINEAR)
                    {
                        return D3D12_FILTER_MAXIMUM_MIN_POINT_MAG_MIP_LINEAR;
                    }
                }
                break;
            }

            case FILTER_TYPE::FILTER_TYPE_MAXIMUM_LINEAR:
            {
                if(magFilter == FILTER_TYPE_MAXIMUM_POINT)
                {
                    if(mipfilter == FILTER_TYPE_MAXIMUM_POINT)
                    {
                        return D3D12_FILTER_MAXIMUM_MIN_LINEAR_MAG_MIP_POINT;
                    }
                    else if(mipfilter == FILTER_TYPE_MAXIMUM_LINEAR)
                    {
                        return D3D12_FILTER_MAXIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
                    }
                }
                else if(magFilter == FILTER_TYPE_MAXIMUM_LINEAR)
                {
                    if(mipfilter == FILTER_TYPE_MAXIMUM_POINT)
                    {
                        return D3D12_FILTER_MAXIMUM_MIN_MAG_LINEAR_MIP_POINT;
                    }
                    else if(mipfilter == FILTER_TYPE_MAXIMUM_LINEAR)
                    {
                        return D3D12_FILTER_MAXIMUM_MIN_MAG_MIP_LINEAR;
                    }
                }
                break;
            }

            case FILTER_TYPE::FILTER_TYPE_MAXIMUM_ANISOTROPIC:
            {
                cyber_assert(magFilter == FILTER_TYPE_MAXIMUM_ANISOTROPIC && mipfilter == FILTER_TYPE_MAXIMUM_ANISOTROPIC, "For anisotropic filtering, both mag and min filters must be anisotropic.");
                return D3D12_FILTER_MAXIMUM_ANISOTROPIC;
                break;
            }

            default:
                CB_CORE_ERROR("Unknown filter type");
                break;
        }

        return D3D12_FILTER_MIN_MAG_MIP_POINT;
    }

    D3D12_TEXTURE_ADDRESS_MODE D3D12Util_TranslateAddressMode(ADDRESS_MODE mode)
    {
        switch (mode) 
        {
            case ADDRESS_MODE_UNKNOWN : CB_ERROR("Unknown address mode"); return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            case ADDRESS_MODE_WRAP : return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            case ADDRESS_MODE_MIRROR : return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
            case ADDRESS_MODE_CLAMP : return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            case ADDRESS_MODE_BORDER : return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
            case ADDRESS_MODE_MIRROR_ONCE : return D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;
            default: CB_ERROR("Unknown address mode"); return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        }
    }

    D3D12_COMPARISON_FUNC D3D12Util_TranslateCompareMode(COMPARE_MODE mode)
    {
        switch(mode)
        {
            case CMP_NEVER: return D3D12_COMPARISON_FUNC_NEVER;
            case CMP_LESS: return D3D12_COMPARISON_FUNC_LESS;
            case CMP_EQUAL: return D3D12_COMPARISON_FUNC_EQUAL;
            case CMP_LESS_EQUAL: return D3D12_COMPARISON_FUNC_LESS_EQUAL;
            case CMP_GREATER: return D3D12_COMPARISON_FUNC_GREATER;
            case CMP_NOT_EQUAL: return D3D12_COMPARISON_FUNC_NOT_EQUAL;
            case CMP_GREATER_EQUAL: return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
            case CMP_ALWAYS: return D3D12_COMPARISON_FUNC_ALWAYS;
            default: CB_ERROR("Unknown compare mode"); return D3D12_COMPARISON_FUNC_NEVER;
        }
    }

    uint32_t d3d12_command_list_type_to_queue_id(D3D12_COMMAND_LIST_TYPE type)
    {
        switch(type)
        {
            case D3D12_COMMAND_LIST_TYPE_DIRECT: return d3d12_queue_index_graphics;
            case D3D12_COMMAND_LIST_TYPE_COMPUTE: return d3d12_queue_index_compute;
            case D3D12_COMMAND_LIST_TYPE_COPY: return d3d12_queue_index_copy;
            default:
            cyber_error("Unknown command list type");
            return 0;
        }
    }
    
#if !defined (XBOX) && defined (_WIN32)
    static D3D12Util_DXCLoader DxcLoader;

    void TestModel()
    {
        auto procDxcCreateInstance = DxcLoader.Get();
        DxcLoader.shader_model_major = 6;
        constexpr char TestShader[] = R"(
        float4 main() : SV_Target0
        {
            return float4(0.0, 0.0, 0.0, 0.0);
        }
        )";

        IDxcLibrary* pLibrary = nullptr;
        IDxcCompiler* pCompiler = nullptr;
        procDxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&pLibrary));
        procDxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&pCompiler));

        IDxcBlobEncoding* pSource = nullptr;
        pLibrary->CreateBlobWithEncodingFromPinned(TestShader, sizeof(TestShader), 0, &pSource);

        eastl::vector<const wchar_t*> DxilArgs;

        for(uint32_t MinorVer = 1;;++MinorVer)
        {
            eastl::wstring Profile(eastl::wstring::CtorSprintf(), L"ps_6_%d" , MinorVer);
            IDxcOperationResult* pdxcResult = nullptr;
            auto hr = pCompiler->Compile(pSource, L"", L"main", Profile.c_str(), !DxilArgs.empty() ? DxilArgs.data() : nullptr, (uint32_t)DxilArgs.size(), nullptr, 0, nullptr, &pdxcResult);
            if(FAILED(hr))
            {
                break;
            }

            HRESULT status = E_FAIL;
            if(FAILED(pdxcResult->GetStatus(&status)))
            {
                break;
            }
            if(FAILED(status))
            {
                break;
            }

            DxcLoader.shader_model_minor = MinorVer;
        }
    }

    void d3d12_util_load_dxc_dll()
    {
        DxcLoader.Load();

        auto procDxcCreateInstance = DxcLoader.Get();
        TestModel();

        if(procDxcCreateInstance)
        {
            IDxcValidator* pValidator = nullptr;
            if(SUCCEEDED(procDxcCreateInstance(CLSID_DxcValidator, IID_PPV_ARGS(&pValidator))))
            {
                IDxcVersionInfo* pVersionInfo;
                if(SUCCEEDED(pValidator->QueryInterface(IID_PPV_ARGS(&pVersionInfo))))
                {
                    pVersionInfo->GetVersion(&DxcLoader.mMajorVersion, &DxcLoader.mMinorVersion);

                }
            }
            CB_INFO("Loaded DX Shader Compiler {0}.{1}. Max supported shader model: {2}.{3}", DxcLoader.mMajorVersion, DxcLoader.mMinorVersion, DxcLoader.shader_model_major, DxcLoader.shader_model_minor);
        }
    }
    void d3d12_util_unload_dxc_dll()
    {
        DxcLoader.Unload();
    }
    DxcCreateInstanceProc d3d12_util_get_dxc_create_instance_proc()
    {
        if(DxcLoader.pDxcCreateInstance == nullptr)
        {
            d3d12_util_load_dxc_dll();
        }
        return DxcLoader.Get();
    }
#endif
}