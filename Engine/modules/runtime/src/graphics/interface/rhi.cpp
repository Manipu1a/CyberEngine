#include "interface/graphics_types.h"
#include "backend/d3d12/rhi_d3d12.h"
#include "platform/memory.h"

namespace Cyber
{
    /*
    RHI* RHI::gDynamicRHI = nullptr;

    void RHI::createRHI(ERHIBackend backend)
    {
        if(backend == ERHIBackend::RHI_BACKEND_D3D12)
        {
            gDynamicRHI = cyber_new<RHI_D3D12>();
            return;
        }

        cyber_assert(false, "RHI for platform [0] initialize failed!", backend);
    }
    */
}