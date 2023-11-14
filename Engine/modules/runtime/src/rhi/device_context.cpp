#include "rhi/device_context.h"

namespace Cyber
{
    namespace RenderObject
    {
        CDeviceContext::CDeviceContext(RHIDevice* device)
        {
            pDevice = device;
        }

        CDeviceContext::~CDeviceContext()
        {

        }

        void CDeviceContext::BeginRenderPass(const RHIRenderPassDesc& desc)
        {

        }

        void CDeviceContext::EndRenderPass()
        {

        }
    }
}