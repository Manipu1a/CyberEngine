#include "rhi/rhi.h"
#include "rhi/backend/d3d12/rhi_d3d12.h"
#include "platform/memory.h"

namespace Cyber
{
    RHI* RHI::createRHI(ERHIBackend backend)
    {

        if(backend == ERHIBackend::RHI_BACKEND_D3D12)
        {
            RHI* context = cyber_new<RHI_D3D12>();
            return context;
        }
        cyber_assert(false, "RHI for platform [0] initialize failed!", backend);
    }
}