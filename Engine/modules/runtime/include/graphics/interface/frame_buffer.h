#pragma once
#include "common/cyber_graphics_config.h"
#include "device_object.h"
#include "platform/memory.h"
namespace Cyber
{
    namespace RenderObject
    {
        struct CYBER_GRAPHICS_API FrameBuffserDesc
        {
            FrameBuffserDesc() {}
            ~FrameBuffserDesc() {}

            const char8_t* m_name;
            class IRenderPass* m_pRenderPass;
            uint32_t m_attachmentCount;
            class ITexture_View** m_ppAttachments;
        };
        
        struct CYBER_GRAPHICS_API IFrameBuffer : public IDeviceObject
        {
            virtual const FrameBuffserDesc& get_create_desc() const = 0;

            virtual ITexture_View* get_attachment(uint32_t index) const = 0;
            virtual IRenderPass* get_render_pass() const = 0;
            virtual void update_attachments(ITexture_View** ppAttachments, uint32_t attachmentCount) = 0;
        };

        template<typename EngineImplTraits>
        class FrameBufferBase : public DeviceObjectBase<typename EngineImplTraits::FrameBufferInterface, typename EngineImplTraits::RenderDeviceImplType>
        {
        public:
            using FrameBufferInterface = typename EngineImplTraits::FrameBufferInterface;
            using RenderDeviceImplType = typename EngineImplTraits::RenderDeviceImplType;
            using TFrameBufferBase = DeviceObjectBase<FrameBufferInterface, RenderDeviceImplType>;

            FrameBufferBase(RenderDeviceImplType* device, const FrameBuffserDesc& desc) : TFrameBufferBase(device), m_desc(desc)
            {
                m_ppAttachments = (ITexture_View**)cyber_malloc(desc.m_attachmentCount * sizeof(ITexture_View*));
                for (uint32_t i = 0; i < desc.m_attachmentCount; ++i)
                {
                    m_ppAttachments[i] = desc.m_ppAttachments[i];
                }
                m_pRenderPass = desc.m_pRenderPass;
            }

            virtual ~FrameBufferBase() = default;

            virtual const FrameBuffserDesc& get_create_desc() const override final
            {
                return m_desc;
            }

            virtual ITexture_View* get_attachment(uint32_t index) const override final
            {
                return m_ppAttachments[index];
            }

            virtual IRenderPass* get_render_pass() const override final
            {
                return m_pRenderPass;
            }

            virtual void update_attachments(ITexture_View** ppAttachments, uint32_t attachmentCount) override
            {  
                for (uint32_t i = 0; i < attachmentCount; ++i)
                {
                    m_ppAttachments[i] = ppAttachments[i];
                }
                m_desc.m_attachmentCount = attachmentCount;
            }
        private:
            FrameBuffserDesc m_desc;
            ITexture_View** m_ppAttachments = nullptr;
            IRenderPass* m_pRenderPass = nullptr;
        };
    }

}