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

    void D3D12Util_Optionalenable_debug_layer(RenderObject::Instance_D3D12_Impl* result, const RenderObject::InstanceCreateDesc& instanceDesc)
    {
        if(instanceDesc.enable_debug_layer)
        {
            if(SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&result->pDXDebug))))
            {
                result->pDXDebug->EnableDebugLayer();
                if(instanceDesc.enable_gpu_based_validation)
                {
                    ID3D12Debug1* pDebug1 = nullptr;
                    if(SUCCEEDED(result->pDXDebug->QueryInterface(IID_PPV_ARGS(&pDebug1))))
                    {
                        pDebug1->SetEnableGPUBasedValidation(instanceDesc.enable_gpu_based_validation);
                        pDebug1->Release();
                    }
                }
            }
        }
        else if(instanceDesc.enable_gpu_based_validation)
        {
            cyber_warn(false, "D3D12 GpuBasedValidation enabled while DebugLayer is closed, there'll be no effect.");
        }
    }

    void D3D12Util_QueryAllAdapters(RenderObject::Instance_D3D12_Impl* instance, uint32_t& pCount, bool& pFoundSoftwareAdapter)
    {
        cyber_assert(instance->pAdapters == nullptr, "getProperGpuCount should be called only once!");
        cyber_assert(instance->mAdaptersCount == 0, "getProperGpuCount should be called only once!");

        IDXGIAdapter* _adapter = nullptr;
        eastl::vector<IDXGIAdapter4*> dxgi_adapters;
        eastl::vector<D3D_FEATURE_LEVEL> adapter_levels;

        for(UINT i = 0; i < 10;i++)
        {
            if(!SUCCEEDED(instance->pDXGIFactory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&_adapter))))
            {
                break;
            }
            DECLARE_ZERO(DXGI_ADAPTER_DESC3, desc)
            IDXGIAdapter4* adapter = nullptr;
            _adapter->QueryInterface(IID_PPV_ARGS(&adapter));
            adapter->GetDesc3(&desc);
            // Ignore Microsoft Driver
            if(!(desc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE))
            {
                uint32_t level_c = RHI_ARRAY_LEN(d3d_feature_levels);

                for(uint32_t level = 0;level < level_c;++level)
                {
                    ID3D12Device* pDevice = nullptr;
                    if(SUCCEEDED(D3D12CreateDevice(adapter, d3d_feature_levels[level], IID_PPV_ARGS(&pDevice))))
                    {
                        struct IDXGIAdapter4* tempAdapter;
                        HRESULT hres = adapter->QueryInterface(IID_PPV_ARGS(&tempAdapter));
                        if(SUCCEEDED(hres))
                        {
                            SAFE_RELEASE(tempAdapter);
                            instance->mAdaptersCount++;
                            // Add ref
                            {
                                dxgi_adapters.push_back(adapter);
                                adapter_levels.push_back(d3d_feature_levels[level]);
                            }
                            break;
                        }
                    }
                }
            }
            else 
            {
                pFoundSoftwareAdapter = true;
                SAFE_RELEASE(adapter);
            }
        }

        pCount = instance->mAdaptersCount;
        instance->pAdapters = cyber_new_n<RenderObject::Adapter_D3D12_Impl>(instance->mAdaptersCount);

        for(uint32_t i = 0;i < pCount; i++)
        {
            RenderObject::Adapter_D3D12_Impl* pAdapter = static_cast<RenderObject::Adapter_D3D12_Impl*>(&instance->pAdapters[i]);
            // Device Objects
            pAdapter->fill_adapter(RenderObject::AdapterDetail{}, adapter_levels[i], dxgi_adapters[i], false);
        }
    }

    void D3D12Util_CreateDescriptorHeap(RenderObject::RenderDevice_D3D12_Impl* device, const D3D12_DESCRIPTOR_HEAP_DESC& pDesc, struct RHIDescriptorHeap_D3D12** ppDescHeap)
    {
        uint32_t numDesciptors = pDesc.NumDescriptors;
        RHIDescriptorHeap_D3D12* pHeap = (RHIDescriptorHeap_D3D12*)cyber_calloc(1, sizeof(*pHeap));
        // TODO thread safety

        pHeap->pDevice = device->GetD3D12Device();

        // Keep 32 aligned for easy remove
        //numDesciptors = cyber_round_up(numDesciptors, 32);

        D3D12_DESCRIPTOR_HEAP_DESC Desc = pDesc;
        Desc.NumDescriptors = numDesciptors;
        pHeap->mDesc = Desc;

        if(!SUCCEEDED(device->GetD3D12Device()->CreateDescriptorHeap(&Desc, IID_ARGS(&pHeap->pCurrentHeap))))
        {
            cyber_assert(false, "DescriptorHeap Create Failed!");
        }
        
        pHeap->mStartHandle.mCpu = pHeap->pCurrentHeap->GetCPUDescriptorHandleForHeapStart();
        if(pHeap->mDesc.Flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE)
        {
            pHeap->mStartHandle.mGpu = pHeap->pCurrentHeap->GetGPUDescriptorHandleForHeapStart();
        }
        pHeap->mDescriptorSize = device->GetD3D12Device()->GetDescriptorHandleIncrementSize(pHeap->mDesc.Type);
        if(Desc.Flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE)
        {
            pHeap->pHandles = (D3D12_CPU_DESCRIPTOR_HANDLE*)cyber_calloc(Desc.NumDescriptors, sizeof(D3D12_CPU_DESCRIPTOR_HANDLE));
        }

        *ppDescHeap = pHeap;
    }

    void D3D12Util_FreeDescriptorHeap(struct RHIDescriptorHeap_D3D12* heap)
    {
        if(heap == nullptr) return;
        SAFE_RELEASE(heap->pCurrentHeap);

        heap->mFreeList.~vector();

        cyber_free(heap->pHandles);
        cyber_free(heap);
    }

    void D3D12Util_InitializeShaderReflection(RenderObject::RenderDevice_D3D12_Impl* device,  RenderObject::ShaderLibrary_D3D12_Impl* library, const RenderObject::ShaderLibraryCreateDesc& desc)
    {
        ID3D12ShaderReflection* d3d12Reflection = nullptr;
        auto procDxcCreateInstance = D3D12Util_GetDxcCreateInstanceProc();

        bool bUseDXC = false;
        switch(desc.shader_compiler)
        {
            case SHADER_COMPILER_DEFAULT:
                bUseDXC = false;
                break;
            case SHADER_COMPILER_GLSLANG:
                bUseDXC = false;
                break;
            case SHADER_COMPILER_DXC:
                bUseDXC = true;
                break;
            case SHADER_COMPILER_FXC:
                bUseDXC = false;
                break;
            default:
                CB_CORE_ERROR("Invalid shader compiler");
                return;
        }

        if(bUseDXC)
        {
            IDxcUtils* pDxcUtils;
            procDxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&pDxcUtils));

            IDxcBlob *pReflectionData;
            library->shader_result->GetOutput(DXC_OUT_REFLECTION, IID_PPV_ARGS(&pReflectionData), nullptr);
            if(pReflectionData != nullptr)
            {
                DxcBuffer ReflectionData;
                ReflectionData.Encoding = DXC_CP_ACP;
                ReflectionData.Ptr = pReflectionData->GetBufferPointer();
                ReflectionData.Size = pReflectionData->GetBufferSize();

                if(SUCCEEDED(pDxcUtils->CreateReflection(&ReflectionData, IID_PPV_ARGS(&d3d12Reflection))))
                {
                    D3D12_SHADER_DESC* shader_desc;
                    d3d12Reflection->GetDesc(shader_desc);
                }
            }
        }
        else 
        {
            D3DReflect(library->shader_blob->GetBufferPointer(), library->shader_blob->GetBufferSize(), __uuidof(d3d12Reflection), reinterpret_cast<void**>(&d3d12Reflection));
        }

        if(d3d12Reflection)
        {
            D3D12Util_CollectShaderReflectionData(d3d12Reflection, desc.stage, library);
        }

        d3d12Reflection->Release();
    }

    void D3D12Util_FreeShaderReflection(RenderObject::ShaderLibrary_D3D12_Impl* library)
    {
        if(library->entry_reflections)
        {
            RenderObject::ShaderReflection_D3D12_Impl* reflection = static_cast<RenderObject::ShaderReflection_D3D12_Impl*>(library->entry_reflections);
            if(reflection->vertex_inputs)
            {
                for(uint32_t i = 0; i < reflection->vertex_input_count; ++i)
                {
                    cyber_free((void*)reflection->vertex_inputs[i].name);
                }
                cyber_free(reflection->vertex_inputs);
            }
            if(reflection->shader_resources)
            {
                for(uint32_t i = 0; i < reflection->shader_resource_count; ++i)
                {
                    cyber_free((void*)reflection->shader_resources[i].name);
                }
                cyber_free(reflection->shader_resources);
            }
            cyber_free(reflection);
        }
    }

    void D3D12Util_SignalFence(RHIQueue_D3D12* queue, ID3D12Fence* fence, uint64_t fenceValue)
    {
        queue->pCommandQueue->Signal(fence, fenceValue);
    }

    void D3D12Util_CreateCBV(RenderObject::RenderDevice_D3D12_Impl* device, const D3D12_CONSTANT_BUFFER_VIEW_DESC* pCbvDesc, D3D12_CPU_DESCRIPTOR_HANDLE* pHandle)
    {
        if(pHandle->ptr == D3D12_GPU_VIRTUAL_ADDRESS_NULL)
            *pHandle = D3D12Util_ConsumeDescriptorHandles(device->GetCPUDescriptorHeaps(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV), 1).mCpu;
        device->GetD3D12Device()->CreateConstantBufferView(pCbvDesc, *pHandle);
    }

    void D3D12Util_CreateSRV(RenderObject::RenderDevice_D3D12_Impl* device, ID3D12Resource* pResource, const D3D12_SHADER_RESOURCE_VIEW_DESC* pSrvDesc, D3D12_CPU_DESCRIPTOR_HANDLE* pHandle)
    {
        if(pHandle->ptr == D3D12_GPU_VIRTUAL_ADDRESS_NULL)
            *pHandle = D3D12Util_ConsumeDescriptorHandles(device->GetCPUDescriptorHeaps(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV), 1).mCpu;
        device->GetD3D12Device()->CreateShaderResourceView(pResource, pSrvDesc, *pHandle);
    }

    void D3D12Util_CreateUAV(RenderObject::RenderDevice_D3D12_Impl* device, ID3D12Resource* pResource, ID3D12Resource* pCounterResource, const D3D12_UNORDERED_ACCESS_VIEW_DESC* pUavDesc, D3D12_CPU_DESCRIPTOR_HANDLE* pHandle)
    {
        if(pHandle->ptr == D3D12_GPU_VIRTUAL_ADDRESS_NULL)
            *pHandle = D3D12Util_ConsumeDescriptorHandles(device->GetCPUDescriptorHeaps(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV), 1).mCpu;
        device->GetD3D12Device()->CreateUnorderedAccessView(pResource, pCounterResource, pUavDesc, *pHandle);
    }

    void D3D12Util_CreateRTV(RenderObject::RenderDevice_D3D12_Impl* device, ID3D12Resource* pResource, const D3D12_RENDER_TARGET_VIEW_DESC* pRtvDesc, D3D12_CPU_DESCRIPTOR_HANDLE* pHandle)
    {
        if(pHandle->ptr == D3D12_GPU_VIRTUAL_ADDRESS_NULL)
            *pHandle = D3D12Util_ConsumeDescriptorHandles(device->GetCPUDescriptorHeaps(D3D12_DESCRIPTOR_HEAP_TYPE_RTV), 1).mCpu;
        device->GetD3D12Device()->CreateRenderTargetView(pResource, pRtvDesc, *pHandle);
    }

    void D3D12Util_CreateDSV(RenderObject::RenderDevice_D3D12_Impl* device, ID3D12Resource* pResource, const D3D12_DEPTH_STENCIL_VIEW_DESC* pDsvDesc, D3D12_CPU_DESCRIPTOR_HANDLE* pHandle)
    {
        if(pHandle->ptr == D3D12_GPU_VIRTUAL_ADDRESS_NULL)
            *pHandle = D3D12Util_ConsumeDescriptorHandles(device->GetCPUDescriptorHeaps(D3D12_DESCRIPTOR_HEAP_TYPE_DSV), 1).mCpu;
        device->GetD3D12Device()->CreateDepthStencilView(pResource, pDsvDesc, *pHandle);
    }

    D3D12_BLEND_DESC D3D12Util_TranslateBlendState(const RHIBlendStateCreateDesc* pDesc)
    {
        int blendDescIndex = 0;
        D3D12_BLEND_DESC ret = {};
        ret.AlphaToCoverageEnable = (BOOL)pDesc->alpha_to_coverage;
        ret.IndependentBlendEnable = TRUE;
        for(int i = 0; i < RHI_MAX_MRT_COUNT;++i)
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
    D3D12_RASTERIZER_DESC D3D12Util_TranslateRasterizerState(const RHIRasterizerStateCreateDesc* pDesc)
    {
        cyber_check(pDesc->fill_mode < RHI_FILL_MODE_COUNT);
        cyber_check(pDesc->cull_mode < RHI_CULL_MODE_COUNT);
        cyber_check(pDesc->front_face == RHI_FRONT_FACE_COUNTER_CLOCKWISE || pDesc->front_face == RHI_FRONT_FACE_CLOCKWISE);
        D3D12_RASTERIZER_DESC ret = {};
        ret.FillMode = gDx12FillModeTranslator[pDesc->fill_mode];
        ret.CullMode = gDx12CullModeTranslator[pDesc->cull_mode];
        ret.FrontCounterClockwise = (pDesc->front_face == RHI_FRONT_FACE_COUNTER_CLOCKWISE);
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

    D3D12_DEPTH_STENCIL_DESC D3D12Util_TranslateDepthStencilState(const RHIDepthStateCreateDesc* pDesc)
    {
        cyber_check(pDesc->depth_func < RHI_CMP_COUNT);
        cyber_check(pDesc->stencil_front_func < RHI_CMP_COUNT);
        cyber_check(pDesc->stencil_front_fail_op < RHI_STENCIL_OP_COUNT);
        cyber_check(pDesc->stencil_front_pass_op < RHI_STENCIL_OP_COUNT);
        cyber_check(pDesc->stencil_front_depth_fail_op < RHI_STENCIL_OP_COUNT);
        cyber_check(pDesc->stencil_back_fail_op < RHI_STENCIL_OP_COUNT);
        cyber_check(pDesc->stencil_back_pass_op < RHI_STENCIL_OP_COUNT);
        cyber_check(pDesc->stencil_back_depth_fail_op < RHI_STENCIL_OP_COUNT);

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

    D3D12_PRIMITIVE_TOPOLOGY_TYPE D3D12Util_TranslatePrimitiveTopologyType(ERHIPrimitiveTopology topology)
    {
        switch(topology)
        {
            case RHI_PRIM_TOPO_POINT_LIST: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
            case RHI_PRIM_TOPO_LINE_LIST:
            case RHI_PRIM_TOPO_LINE_STRIP: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
            case RHI_PRIM_TOPO_TRIANGLE_LIST:
            case RHI_PRIM_TOPO_TRIANGLE_STRIP: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
            case RHI_PRIM_TOPO_PATCH_LIST: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
        }
        cyber_check(false);
        return D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
    }
}