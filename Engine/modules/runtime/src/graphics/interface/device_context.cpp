#include "interface/device_context.h"

namespace Cyber
{
    namespace RenderObject
    {
        CEDeviceContext::CEDeviceContext(IRenderDevice* device)
        {
            m_pRenderDevice = device;
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