#include "graphics/backend/d3d12/device_context_d3d12.h"
#include "graphics/backend/d3d12//render_device_d3d12.h"
CYBER_BEGIN_NAMESPACE(Cyber)
CYBER_BEGIN_NAMESPACE(RenderObject)

DeviceContext_D3D12_Impl::DeviceContext_D3D12_Impl(RenderDevice_D3D12_Impl* device, const DeviceContextDesc& desc)
    : DeviceContextBase<EngineD3D12ImplTraits>(device, desc)
{
    request_command_context();
}

void DeviceContext_D3D12_Impl::request_command_context()
{
    command_context = render_device->allocate_command_context(get_command_queue_id());
}

CYBER_END_NAMESPACE
CYBER_END_NAMESPACE