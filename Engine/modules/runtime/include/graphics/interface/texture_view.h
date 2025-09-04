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
            virtual ITexture* get_texture() = 0;
            virtual void* get_gpu_native_resource() = 0;
            virtual bool operator==(const ITexture_View& other) = 0;
            virtual bool operator!=(const ITexture_View& other) = 0;
        };

        template<typename EngineImplTraits>
        class CYBER_GRAPHICS_API Texture_View : public DeviceObjectBase<typename EngineImplTraits::TextureViewInterface, typename EngineImplTraits::RenderDeviceImplType>
        {
        public:
            using RenderDeviceImplType = typename EngineImplTraits::RenderDeviceImplType;
            using TextureViewInterface = typename EngineImplTraits::TextureViewInterface;
            using TTexture_View =  DeviceObjectBase<TextureViewInterface, RenderDeviceImplType>;

            Texture_View(RenderDeviceImplType* device, const TextureViewCreateDesc& desc) : TTexture_View(device), m_desc(desc)
            {
            }
            virtual ~Texture_View() {}
            
            virtual const TextureViewCreateDesc& get_create_desc() override
            {
                return m_desc;
            }

            virtual void* get_gpu_native_resource() override
            {
                return m_desc.p_native_resource;
            }

            virtual ITexture* get_texture() override
            {
                return m_desc.p_texture;
            }
            
            bool operator==(const ITexture_View& other) override
            {
                const Texture_View* other_view = static_cast<const Texture_View*>(&other);
                return m_desc.p_texture == other_view->m_desc.p_texture && 
                        m_desc.format == other_view->m_desc.format &&
                       m_desc.view_type == other_view->m_desc.view_type &&
                       m_desc.aspects == other_view->m_desc.aspects &&
                       m_desc.dimension == other_view->m_desc.dimension &&
                       m_desc.baseArrayLayer == other_view->m_desc.baseArrayLayer &&
                       m_desc.arrayLayerCount == other_view->m_desc.arrayLayerCount &&
                       m_desc.baseMipLevel == other_view->m_desc.baseMipLevel &&
                       m_desc.mipLevelCount == other_view->m_desc.mipLevelCount;
            }

            bool operator!=(const ITexture_View& other) override
            {
                return !(*this == other);
            }
        protected:
            TextureViewCreateDesc m_desc;

            friend RenderDeviceImplType;
        };
    }
}
