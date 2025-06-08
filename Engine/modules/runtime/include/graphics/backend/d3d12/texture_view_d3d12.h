#pragma once

#include "graphics/interface/texture_view.h"
#include "engine_impl_traits_d3d12.hpp"

CYBER_BEGIN_NAMESPACE(Cyber)
CYBER_BEGIN_NAMESPACE(RenderObject)

class RenderDevice_D3D12_Impl;
struct ITexture_View_D3D12 : public ITexture_View
{
};

class Texture_View_D3D12_Impl : public Texture_View<EngineD3D12ImplTraits>
{
public:
    using RenderDeviceImplType = EngineD3D12ImplTraits::RenderDeviceImplType;
    using TTextureViewBase = Texture_View<EngineD3D12ImplTraits>;
    Texture_View_D3D12_Impl(class RenderDevice_D3D12_Impl* device, const TextureViewCreateDesc& desc) : TTextureViewBase(device, desc) {  }
    virtual ~Texture_View_D3D12_Impl();
protected:
    D3D12_CPU_DESCRIPTOR_HANDLE m_dxDescriptorHandles;
    /// Offset from mDxDescriptors for srv descriptor handle
    uint8_t m_srvDescriptorOffset;
    /// Offset from mDxDescriptors for uav descriptor handle
    uint8_t m_uavDescriptorOffset;
    /// Offset from mDxDescriptors for rtv descriptor handle
    D3D12_CPU_DESCRIPTOR_HANDLE m_rtvDsvDescriptorHandle;
    friend class RenderObject::RenderDevice_D3D12_Impl;
    friend class RenderObject::DeviceContext_D3D12_Impl;
};

CYBER_END_NAMESPACE
CYBER_END_NAMESPACE