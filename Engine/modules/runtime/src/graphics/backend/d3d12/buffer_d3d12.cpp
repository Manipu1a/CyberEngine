#include "graphics/backend/d3d12/buffer_d3d12.h"

CYBER_BEGIN_NAMESPACE(Cyber)
CYBER_BEGIN_NAMESPACE(RenderObject)

IBuffer_View* Buffer_D3D12_Impl::create_view_internal(const BufferViewCreateDesc& desc) const
{
    auto* device = get_device();
    auto* view = device->create_buffer_view(desc);
    
    if(view == nullptr)
    {
        cyber_assert(false, "Failed to create buffer view");
        return nullptr;
    }

    return view;
}

void Buffer_D3D12_Impl::get_buffer_alignment(const BufferCreateDesc& desc, uint32_t& alignment)
{
    alignment = (desc.structStride > 0) && (desc.bind_flags | GRAPHICS_RESOURCE_BIND_STRUCT_BUFFER) ? least_common_multiple(desc.structStride, 16) : 16;
}

CYBER_END_NAMESPACE
CYBER_END_NAMESPACE