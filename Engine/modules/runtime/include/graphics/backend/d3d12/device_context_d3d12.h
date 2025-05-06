#pragma once
#include "graphics/interface/device_context.h"
#include "graphics/backend/d3d12/command_context.h"
#include "engine_impl_traits_d3d12.hpp"

CYBER_BEGIN_NAMESPACE(Cyber)
CYBER_BEGIN_NAMESPACE(RenderObject)

struct IDeviceContext_D3D12 : public IDeviceContext 
{
    // Interface for D3D12 device context
};

class CYBER_GRAPHICS_API DeviceContext_D3D12_Impl final : public DeviceContextBase<EngineD3D12ImplTraits>
{
public:
    using RenderDeviceImplType = EngineD3D12ImplTraits::RenderDeviceImplType;

    DeviceContext_D3D12_Impl(class RenderDevice_D3D12_Impl* device, const DeviceContextDesc& desc);

    CommandContext& get_command_context()
    {
        return *command_context;
    }
    
    void request_command_context();

private:
    CommandContext* command_context = nullptr;
};

CYBER_END_NAMESPACE
CYBER_END_NAMESPACE