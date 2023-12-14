#pragma once
#include "cyber_rhi_config.h"
#include "rhi.h"
#include "render_pass.h"

namespace Cyber
{
    namespace RenderObject
    {
        struct CYBER_RHI_API FrameBuffserDesc
        {
            const char8_t* name;
            CERenderPass render_pass;
            uint32_t attachment_count;
            Cyber::RHITextureView* attachments;
        };

        class CEFrameBuffer
        {
        public:
            
        };
    }

}
