#pragma once
#include "common/cyber_graphics_config.h"
#include "interface/texture_view.h"
#include "render_pass.h"

namespace Cyber
{
    namespace RenderObject
    {
        struct CYBER_GRAPHICS_API FrameBuffserDesc
        {
            const char8_t* name;
            CERenderPass* render_pass;
            uint32_t attachment_count;
            RenderObject::ITextureView* attachments;
        };

        class CEFrameBuffer
        {
        public:
            
        };
    }

}