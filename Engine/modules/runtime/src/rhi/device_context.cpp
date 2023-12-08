#include "rhi/device_context.h"

namespace Cyber
{
    namespace RenderObject
    {
        CEDeviceContext::CEDeviceContext(RHIDevice* device)
        {
            pDevice = device;
        }

        CEDeviceContext::~CEDeviceContext()
        {

        }

        void CEDeviceContext::BeginRenderPass(const RHIRenderPassDesc& desc)
        {

        }

        void CEDeviceContext::EndRenderPass()
        {

        }
    }
}