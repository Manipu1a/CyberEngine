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
protected:

};

CYBER_END_NAMESPACE
CYBER_END_NAMESPACE