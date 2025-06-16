#pragma once
#include "graphics/interface/buffer_view.h"
#include "engine_impl_traits_d3d12.hpp"

CYBER_BEGIN_NAMESPACE(Cyber)
CYBER_BEGIN_NAMESPACE(RenderObject)

struct IBuffer_View_D3D12 : public IBuffer_View
{
};

class Buffer_View_D3D12_Impl : public Buffer_View<EngineD3D12ImplTraits>
{
public:
    using RenderDeviceImplType = EngineD3D12ImplTraits::RenderDeviceImplType;
    using TBufferViewBase = Buffer_View<EngineD3D12ImplTraits>;

    Buffer_View_D3D12_Impl(RenderDeviceImplType* device, const BufferViewCreateDesc& desc) : TBufferViewBase(device, desc) { }

    virtual ~Buffer_View_D3D12_Impl() = default;
    
    D3D12_CPU_DESCRIPTOR_HANDLE get_dx_descriptor_handles() const { return m_dxDescriptorHandles; }
    void set_dx_descriptor_handles(D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandles) { m_dxDescriptorHandles = descriptorHandles; }

    uint8_t get_srv_descriptor_offset() const { return m_srvDescriptorOffset; }
    void set_srv_descriptor_offset(uint8_t offset) { m_srvDescriptorOffset = offset; }

    uint8_t get_uav_descriptor_offset() const { return m_uavDescriptorOffset; }
    void set_uav_descriptor_offset(uint8_t offset) { m_uavDescriptorOffset = offset; }

protected:
    /// handle store order: cbv, srv, uav
    /// Descriptor handle of the CBV in a CPU visible descriptor heap (applicable to BUFFER_USAGE_UNIFORM)
    D3D12_CPU_DESCRIPTOR_HANDLE m_dxDescriptorHandles;
    /// Offset from mDxDescriptors for srv descriptor handle
    uint8_t m_srvDescriptorOffset;
    /// Offset from mDxDescriptors for uav descriptor handle
    uint8_t m_uavDescriptorOffset;
    friend class RenderDevice_D3D12_Impl;
};

CYBER_END_NAMESPACE
CYBER_END_NAMESPACE