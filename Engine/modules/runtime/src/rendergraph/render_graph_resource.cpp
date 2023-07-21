#include "rendergraph/render_graph_resource.h"

namespace Cyber
{
    namespace render_graph
    {
        RHIBuffer* RGBuffer::GetBuffer()
        {
            if(buffer)
            {
                return buffer;
            }

            buffer = rhi_create_buffer(device, create_desc);
            if(!buffer)
            {
                cyber_assert(false, "Failed to create buffer");
                return nullptr;
            }
            return buffer;
        }

        RHITexture* RGTexture::GetTexture()
        {
            if(texture)
            {
                return texture;
            }

            texture = rhi_create_texture(device, create_desc);
            if(!texture)
            {
                cyber_assert(false, "Failed to create texture");
                return nullptr;
            }
            return texture;
        }


        /////////////////////////////////////////////////
        void RGRenderPass::execute()
        {
            
        }
    }
}