#pragma once
#include "common/flags.h"
#include "common/cyber_graphics_config.h"
#include "device_object.h"
#include "interface/graphics_types.h"

namespace Cyber
{
    namespace RenderObject
    {
        
        struct CYBER_GRAPHICS_API TextureViewCreateDesc
        {
            const char8_t* m_name;
            class ITexture* m_pTexture;
            void* m_pNativeResource;
            TEXTURE_FORMAT m_format;
            TEXTURE_VIEW_USAGE m_usages;
            TEXTURE_VIEW_ASPECT m_aspects;
            TEXTURE_DIMENSION m_dimension;
            uint32_t m_baseArrayLayer;
            uint32_t m_arrayLayerCount;
            uint32_t m_baseMipLevel;
            uint32_t m_mipLevelCount;
        };

        //class CERenderDevice;
        struct CYBER_GRAPHICS_API ITextureView : public IDeviceObject
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
