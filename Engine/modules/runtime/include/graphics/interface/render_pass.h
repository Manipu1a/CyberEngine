#pragma once
#include "common/cyber_graphics_config.h"
#include "common/flags.h"
#include "device_object.h"
#include "interface/graphics_types.h"

namespace Cyber
{
    namespace RenderObject
    {
        struct CYBER_GRAPHICS_API RenderPassAttachmentDesc
        {
            TEXTURE_FORMAT m_format;
            uint8_t m_sampleCount;
            LOAD_ACTION m_loadAction;
            STORE_ACTION m_storeAction;
            LOAD_ACTION m_stencilLoadAction;
            STORE_ACTION m_stencilStoreAction;
            GRAPHICS_RESOURCE_STATE m_initialState;
            GRAPHICS_RESOURCE_STATE m_finalState;
        };

        struct CYBER_GRAPHICS_API AttachmentReference
        {
            uint32_t m_attachmentIndex;
            TEXTURE_SAMPLE_COUNT m_sampleCount;
            LOAD_ACTION m_loadAction;
            STORE_ACTION m_storeAction;
            LOAD_ACTION m_stencilLoadAction;
            STORE_ACTION m_stencilStoreAction;
            GRAPHICS_RESOURCE_STATE m_initialState;
            GRAPHICS_RESOURCE_STATE m_finalState;
        };

        struct CYBER_GRAPHICS_API RenderSubpassDesc
        {
            //TEXTURE_SAMPLE_COUNT m_sampleCount;
            uint32_t m_inputAttachmentCount;
            const AttachmentReference* m_pInputAttachments;
            const AttachmentReference* m_pDepthStencilAttachment;
            uint32_t m_renderTargetCount;
            const AttachmentReference* m_pRenderTargetAttachments;
        };

        struct CYBER_GRAPHICS_API SubpassDependencyDesc
        {
            uint32_t m_srcSubpass;
            uint32_t m_dstSubpass;
            PIPELINE_STAGE_FLAG m_srcStageMask;
            PIPELINE_STAGE_FLAG m_dstStageMask;
            ACCESS_FLAG m_srcAccessMask;
            ACCESS_FLAG m_dstAccessMask;
        };

        struct CYBER_GRAPHICS_API RenderPassDesc
        {
            const char8_t* m_name;
            uint32_t m_attachmentCount;
            const RenderPassAttachmentDesc* m_pAttachments;
            uint32_t m_subpassCount;
            const char8_t* const* m_subpassNames;
            RenderSubpassDesc* m_pSubpasses;
        };

        struct CYBER_GRAPHICS_API IRenderPass : public IDeviceObject
        {
            virtual const RenderPassDesc& get_create_desc() const = 0;
        };

        template<typename EngineImplTraits>
        class CYBER_GRAPHICS_API RenderPassBase : public DeviceObjectBase<typename EngineImplTraits::RenderPassInterface, typename EngineImplTraits::RenderDeviceImplType>
        {
        public:
            using RenderPassInterface = typename EngineImplTraits::RenderPassInterface;
            using RenderDeviceImplType = typename EngineImplTraits::RenderDeviceImplType;
        using TRenderPassBase = typename DeviceObjectBase<RenderPassInterface, RenderDeviceImplType>;

            RenderPassBase(RenderDeviceImplType* device, const RenderPassDesc& desc) : TRenderPassBase(device), m_desc(desc)
            {
            }

            virtual const RenderPassDesc& get_create_desc() const override { return m_desc; }
        protected:
            RenderPassDesc m_desc;
        };
    }
}
