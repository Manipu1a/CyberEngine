#pragma once
#include "rhi/rhi.h"

namespace Cyber
{
    namespace render_graph
    {
        struct RenderPassContext
        {
        public:
            RHIRenderPassEncoder* encoder;
        };
    }
}
