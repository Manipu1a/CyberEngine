#include "graphics/backend/d3d12/root_signature_d3d12.h"

CYBER_BEGIN_NAMESPACE(Cyber)
CYBER_BEGIN_NAMESPACE(RenderObject)

RootSignature_D3D12_Impl::~RootSignature_D3D12_Impl()
{
    if (dxRootSignature)
    {
        dxRootSignature->Release();
        dxRootSignature = nullptr;
    }
}
CYBER_END_NAMESPACE
CYBER_END_NAMESPACE