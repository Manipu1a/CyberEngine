#include "interface/device_context.h"

namespace Cyber
{
    namespace RenderObject
    {
        CEDeviceContext::CEDeviceContext(CERenderDevice* device)
        {
            render_device = device;
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