#pragma once
#include "common/cyber_graphics_config.h"
#include "render_object.h"
namespace Cyber
{
    namespace RenderObject
    {
        struct CYBER_GRAPHICS_API FrameBuffserDesc
        {
            const char8_t* m_name;
            class IRenderPass* m_pRenderPass;
            uint32_t m_attachmentCount;
            class ITextureView** m_ppAttachments;
        };
        
        struct CYBER_GRAPHICS_API IFrameBuffer
        {
            virtual const FrameBuffserDesc& get_create_desc() const = 0;
        };

        template<typename EngineImplTraits>
        class FrameBufferBase : public RenderObjectBase<typename EngineImplTraits::FrameBufferInterface, typename EngineImplTraits::RenderDeviceImplType>
        {
        public:
            using FrameBufferInterface = typename EngineImplTraits::FrameBufferInterface;
            using RenderDeviceImplType = typename EngineImplTraits::RenderDeviceImplType;
            using TFrameBufferBase = typename FrameBufferBase<FrameBufferInterface, RenderDeviceImplType>;

            FrameBufferBase(RenderDeviceImplType* device, const FrameBuffserDesc& desc) : TFrameBufferBase(device), m_desc(desc)
            {
                m_ppAttachments = desc.m_ppAttachments;
                m_pRenderPass = desc.m_pRenderPass;
            }

            virtual ~FrameBufferBase() = default;

            virtual const FrameBuffserDesc& get_create_desc() const
            {
                return m_desc;
            }
        private:
            FrameBuffserDesc m_desc;
            ITextureView** m_ppAttachments = nullptr;
            IRenderPass* m_pRenderPass = nullptr;
        };
    }

}