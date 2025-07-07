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
            const char8_t* m_name;
            uint32_t m_inputAttachmentCount = 0;
            AttachmentReference* m_pInputAttachments = nullptr;
            AttachmentReference* m_pDepthStencilAttachment = nullptr;
            uint32_t m_renderTargetCount = 0;
            AttachmentReference* m_pRenderTargetAttachments = nullptr;
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
            RenderPassAttachmentDesc* m_pAttachments;
            uint32_t m_subpassCount;
            const char8_t* const* m_subpassNames;
            RenderSubpassDesc* m_pSubpasses;
        };

        struct CYBER_GRAPHICS_API IRenderPass : public IDeviceObject
        {
            virtual const RenderPassDesc& get_create_desc() const = 0;
            virtual uint32_t get_subpass_index() const = 0;
            virtual void next_subpass() = 0;
        };

        template<typename EngineImplTraits>
        class CYBER_GRAPHICS_API RenderPassBase : public DeviceObjectBase<typename EngineImplTraits::RenderPassInterface, typename EngineImplTraits::RenderDeviceImplType>
        {
        public:
            using RenderPassInterface = typename EngineImplTraits::RenderPassInterface;
            using RenderDeviceImplType = typename EngineImplTraits::RenderDeviceImplType;
        using TRenderPassBase = typename DeviceObjectBase<RenderPassInterface, RenderDeviceImplType>;

            RenderPassBase(RenderDeviceImplType* device, const RenderPassDesc& desc) : TRenderPassBase(device)
            {
                m_subpassIndex = 0;
                m_desc.m_name = desc.m_name;
                m_desc.m_attachmentCount = desc.m_attachmentCount;
                m_desc.m_pAttachments = (RenderPassAttachmentDesc*)cyber_malloc(desc.m_attachmentCount * sizeof(RenderPassAttachmentDesc));
                for (uint32_t i = 0; i < desc.m_attachmentCount; ++i)
                {
                    m_desc.m_pAttachments[i].m_format = desc.m_pAttachments[i].m_format;
                }
                m_desc.m_subpassCount = desc.m_subpassCount;
                m_desc.m_pSubpasses = (RenderSubpassDesc*)cyber_malloc(desc.m_subpassCount * sizeof(RenderSubpassDesc));
                for (uint32_t i = 0; i < desc.m_subpassCount; ++i)
                {
                    const RenderSubpassDesc& subpassDesc = desc.m_pSubpasses[i];
                    m_desc.m_pSubpasses[i].m_name = subpassDesc.m_name;
                    m_desc.m_pSubpasses[i].m_inputAttachmentCount = subpassDesc.m_inputAttachmentCount;
                    m_desc.m_pSubpasses[i].m_pInputAttachments = (AttachmentReference*)cyber_malloc(subpassDesc.m_inputAttachmentCount * sizeof(AttachmentReference));
                    for (uint32_t j = 0; j < subpassDesc.m_inputAttachmentCount; ++j)
                    {
                        m_desc.m_pSubpasses[i].m_pInputAttachments[j] = subpassDesc.m_pInputAttachments[j];
                    }
                    m_desc.m_pSubpasses[i].m_pDepthStencilAttachment = nullptr;
                    if(subpassDesc.m_pDepthStencilAttachment)
                    {
                        m_desc.m_pSubpasses[i].m_pDepthStencilAttachment = cyber_new<AttachmentReference>();
                        *m_desc.m_pSubpasses[i].m_pDepthStencilAttachment = *subpassDesc.m_pDepthStencilAttachment;
                    }
                    m_desc.m_pSubpasses[i].m_renderTargetCount = subpassDesc.m_renderTargetCount;
                    m_desc.m_pSubpasses[i].m_pRenderTargetAttachments = (AttachmentReference*)cyber_malloc(subpassDesc.m_renderTargetCount * sizeof(AttachmentReference));
                    for (uint32_t j = 0; j < subpassDesc.m_renderTargetCount; ++j)
                    {
                        m_desc.m_pSubpasses[i].m_pRenderTargetAttachments[j] = subpassDesc.m_pRenderTargetAttachments[j];
                    }
                }
            }

            virtual const RenderPassDesc& get_create_desc() const override { return m_desc; }

            virtual uint32_t get_subpass_index() const override { return m_subpassIndex; }
            virtual void next_subpass() override { m_subpassIndex++; }
        protected:
            RenderPassDesc m_desc;
            uint32_t m_subpassIndex;
        };
    }
}
