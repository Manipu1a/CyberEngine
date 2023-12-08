#include "rhi/backend/d3d12/rhi_d3d12.h"
#include "EASTL/vector.h"
#include <EASTL/hash_map.h>
#include <EASTL/string_hash_map.h>
#include "d3d12_utils.h"
#include "CyberLog/Log.h"
#include "dxcapi.h"
#include "math/common.h"
#include <combaseapi.h>
#include <d3d12.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#include <dxgi1_2.h>
#include <dxgi1_5.h>
#include <intsafe.h>
#include <stdint.h>
#include <synchapi.h>
#include "platform/memory.h"
#include "../../common/common_utils.h"

namespace Cyber
{
    HRESULT RHIDevice_D3D12::hook_CheckFeatureSupport(D3D12_FEATURE pFeature, void* pFeatureSupportData, UINT pFeatureSupportDataSize)
    {
        return pDxDevice->CheckFeatureSupport(pFeature, pFeatureSupportData, pFeatureSupportDataSize);
    }

    HRESULT RHIDevice_D3D12::hook_CreateCommittedResource(const D3D12_HEAP_PROPERTIES* pHeapProperties, D3D12_HEAP_FLAGS HeapFlags, const D3D12_RESOURCE_DESC* pDesc, D3D12_RESOURCE_STATES InitialResourceState, const D3D12_CLEAR_VALUE* pOptimizedClearValue, REFIID riidResource, void **ppvResource)
    {
        return pDxDevice->CreateCommittedResource(pHeapProperties, HeapFlags, pDesc, InitialResourceState, pOptimizedClearValue, riidResource, ppvResource);
    }

}