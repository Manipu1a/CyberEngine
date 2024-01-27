#pragma once
#include "common/cyber_graphics_config.h"
#include "render_object.h"
namespace Cyber
{
    namespace RenderObject
    {
        struct CYBER_GRAPHICS_API FrameBuffserDesc
        {
            const char8_t* name;
            class IRenderPass* render_pass;
            uint32_t attachment_count;
            class ITextureView** attachments;
        };
        
        struct CYBER_GRAPHICS_API IFrameBuffer
        {
            virtual const FrameBuffserDesc& get_create_desc() const = 0;
        };

        template<typename EngineImplTraits>
        class FrameBufferBase : public RenderObjectBase<typename EngineImplTraits::FrameBufferInterface, typename EngineImplTraits::RenderDeviceImplType>
        {
        public:
            using RenderDeviceImplType = typename EngineImplTraits::RenderDeviceImplType;

            FrameBufferBase(RenderDeviceImplType* device);

            virtual ~FrameBufferBase() = default;

            virtual const FrameBuffserDesc& get_create_desc() const
            {
                return create_desc;
            }
        private:
            FrameBuffserDesc create_desc;
            ITextureView** attachments = nullptr;
            IRenderPass* render_pass = nullptr;
        };
    }

}