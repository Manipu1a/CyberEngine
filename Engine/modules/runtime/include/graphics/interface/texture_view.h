#pragma once
#include "common/cyber_graphics_config.h"
#include "device_object.h"
#include "interface/graphics_types.h"

namespace Cyber
{
    namespace RenderObject
    {
        
        struct CYBER_GRAPHICS_API TextureViewCreateDesc
        {
            const char8_t* name;
            class ITexture* p_texture;
            void* p_native_resource;
            TEXTURE_FORMAT format;
            uint32_t view_type;
            TEXTURE_VIEW_ASPECT aspects;
            TEXTURE_DIMENSION dimension;
            uint32_t baseArrayLayer;
            uint32_t arrayLayerCount;
            uint32_t baseMipLevel;
            uint32_t mipLevelCount;
        };

        //class CERenderDevice;
        struct CYBER_GRAPHICS_API ITexture_View : public IDeviceObject
        { 
            virtual const TextureViewCreateDesc& get_create_desc() = 0;
        };

        template<typename EngineImplTraits>
        class CYBER_GRAPHICS_API Texture_View : public DeviceObjectBase<typename EngineImplTraits::TextureViewInterface, typename EngineImplTraits::RenderDeviceImplType>
        {
        public:
            using RenderDeviceImplType = typename EngineImplTraits::RenderDeviceImplType;
            using TextureViewInterface = typename EngineImplTraits::TextureViewInterface;
            using TTexture_View = typename DeviceObjectBase<TextureViewInterface, RenderDeviceImplType>;

            Texture_View(RenderDeviceImplType* device, const TextureViewCreateDesc& desc) : TTexture_View(device), m_desc(desc)
            {
            }
            virtual ~Texture_View() {}
            
            virtual const TextureViewCreateDesc& get_create_desc() override
            {
                return m_desc;
            }
        protected:
            TextureViewCreateDesc m_desc;

            friend RenderDeviceImplType;
        };
    }
}
