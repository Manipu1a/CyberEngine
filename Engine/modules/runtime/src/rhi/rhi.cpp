#include "rhi/rhi.h"
//#include "rhi/backend/d3d12/rhi_d3d12.h"

namespace Cyber
{
    RHI* RHI::gloablRHI = nullptr;

   void RHI::createRHI(ERHIBackend backend)
    {
        gloablRHI = nullptr;
        
        if(backend == ERHIBackend::RHI_BACKEND_D3D12)
        {
            ////gloablRHI = new RHI_D3D12();
        }

        cyber_assert(gloablRHI, "RHI for platform [0] initialize failed!", backend);
    }
}