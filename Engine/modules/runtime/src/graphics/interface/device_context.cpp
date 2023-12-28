#include "interface/device_context.h"

namespace Cyber
{
    namespace RenderObject
    {
        CEDeviceContext::CEDeviceContext(IRenderDevice* device)
        {
            render_device = device;
        }

        CEDeviceContext::~CEDeviceContext()
        {

        }

        void CEDeviceContext::BeginRenderPass(const RenderPassDesc& desc)
        {

        }

        void CEDeviceContext::EndRenderPass()
        {

        }
    }
}