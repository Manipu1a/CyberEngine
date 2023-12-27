#pragma once
#include "common/cyber_graphics_config.h"
#include "common/flags.h"

namespace Cyber
{
    namespace RenderObject
    {
        struct CYBER_GRAPHICS_API RenderPassAttachmentDesc
        {
            ERHIFormat format;
            uint8_t sample_count;
            ERHILoadAction load_action;
            ERHIStoreAction store_action;
            ERHILoadAction stencil_load_action;
            ERHIStoreAction stencil_store_action;
            ERHIResourceState initial_state;
            ERHIResourceState final_state;
        };

        struct CYBER_GRAPHICS_API AttachmentReference
        {
            uint32_t attachment_index;
            ERHIResourceState state;
        };

        struct CYBER_GRAPHICS_API RenderSubpassDesc
        {
            ERHITextureSampleCount sample_count;
            uint32_t input_attachment_count;
            const AttachmentReference* input_attachments;
            const AttachmentReference* depth_stencil_attachment;
            uint32_t render_target_count;
            const AttachmentReference* render_target_attachments;
        };

        struct CYBER_GRAPHICS_API SubpassDependencyDesc
        {
            uint32_t src_subpass;
            uint32_t dst_subpass;
            ERHIPipelineStageFlags src_stage_mask;
            ERHIPipelineStageFlags dst_stage_mask;
            ERHIAccessFlags src_access_mask;
            ERHIAccessFlags dst_access_mask;
        };

        struct CYBER_GRAPHICS_API RenderPassDesc
        {
            const char8_t* name;
            uint32_t attachment_count;
            const RenderPassAttachmentDesc* attachments;
            uint32_t subpass_count;
            const char8_t* const* subpass_names;
            RenderSubpassDesc* subpasses;
        };

        class CYBER_GRAPHICS_API CERenderPass
        {
        public:
            
            const RenderPassDesc& get_create_desc() const { return desc; }
            
        protected:
            RenderPassDesc desc;
        };
    }
}
