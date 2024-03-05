#pragma once
#include "common/cyber_graphics_config.h"
#include "common/flags.h"
#include "render_object.h"
namespace Cyber
{
    namespace RenderObject
    {
        struct CYBER_GRAPHICS_API RenderPassAttachmentDesc
        {
            TEXTURE_FORMAT m_Format;
            uint8_t m_SampleCount;
            LOAD_ACTION m_LoadAction;
            STORE_ACTION m_StoreAction;
            LOAD_ACTION m_StencilLoadAction;
            STORE_ACTION m_StencilStoreAction;
            GRAPHICS_RESOURCE_STATE m_InitialState;
            GRAPHICS_RESOURCE_STATE m_FinalState;
        };

        struct CYBER_GRAPHICS_API AttachmentReference
        {
            uint32_t m_AttachmentIndex;
            GRAPHICS_RESOURCE_STATE m_State;
        };

        struct CYBER_GRAPHICS_API RenderSubpassDesc
        {
            TEXTURE_SAMPLE_COUNT m_SampleCount;
            uint32_t m_InputAttachmentCount;
            const AttachmentReference* m_pInputAttachments;
            const AttachmentReference* m_pDepthStencilAttachment;
            uint32_t m_RenderTargetCount;
            const AttachmentReference* m_pRenderTargetAttachments;
        };

        struct CYBER_GRAPHICS_API SubpassDependencyDesc
        {
            uint32_t m_SrcSubpass;
            uint32_t m_DstSubpass;
            PIPELINE_STAGE_FLAG m_SrcStageMask;
            PIPELINE_STAGE_FLAG m_DstStageMask;
            ACCESS_FLAG m_SrcAccessMask;
            ACCESS_FLAG m_DstAccessMask;
        };

        struct CYBER_GRAPHICS_API RenderPassDesc
        {
            const char8_t* m_Name;
            uint32_t m_AttachmentCount;
            const RenderPassAttachmentDesc* m_pAttachments;
            uint32_t m_SubpassCount;
            const char8_t* const* m_SubpassNames;
            RenderSubpassDesc* m_pSubpasses;
        };

        struct CYBER_GRAPHICS_API IRenderPass
        {
            virtual const RenderPassDesc& get_create_desc() const = 0;
        };

        template<typename EngineImplTraits>
        class CYBER_GRAPHICS_API RenderPassBase : public RenderObjectBase<typename EngineImplTraits::TextureInterface, typename EngineImplTraits::RenderDeviceImplType>
        {
        public:
            
            virtual const RenderPassDesc& get_create_desc() const override { return desc; }
        protected:
            RenderPassDesc desc;
        };
    }
}
