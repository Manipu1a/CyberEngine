#pragma once
#include "common/cyber_graphics_config.h"
#include "common/flags.h"
#include "texture_view.h"
#include "device_object.h"
#include "interface/graphics_types.h"

namespace Cyber
{
    namespace RenderObject
    {
        struct CYBER_GRAPHICS_API TextureCreateDesc
        {
            /// Pointer to native texture handle if the texture does not own underlying resource
            void* m_pNativeHandle;
            /// Debug name used in gpu profile
            const char8_t* m_name;
            uint32_t m_width;
            uint32_t m_height;
            uint32_t m_depth;
            uint32_t m_arraySize;
            uint32_t m_mipLevels;
            /// Optimized clear value (recommended to use this same value when clearing the rendertarget)
            GRAPHICS_CLEAR_VALUE m_clearValue;
            DESCRIPTOR_TYPE m_descriptors;
            TEXTURE_CREATE_FLAG m_flags;
            /// Number of multisamples per pixel (currently Textures created with mUsage TEXTURE_USAGE_SAMPLED_IMAGE only support SAMPLE_COUNT_1)
            TEXTURE_SAMPLE_COUNT m_sampleCount;
            /// The image quality level. The higher the quality, the lower the performance. The valid range is between zero and the value appropriate for mSampleCount
            uint32_t m_sampleQuality;
            /// Image format
            TEXTURE_FORMAT m_format;
            /// What state will the texture get created in
            GRAPHICS_RESOURCE_STATE m_startState;
        };


        struct CYBER_GRAPHICS_API ITexture : public IDeviceObject
        {
            virtual const TextureCreateDesc& get_create_desc() const = 0;
            virtual ITextureView* get_default_texture_view(TEXTURE_VIEW_USAGE view_type) const = 0;
        };

        template<typename EngineImplTraits>
        class CYBER_GRAPHICS_API Texture : public DeviceObjectBase<typename EngineImplTraits::TextureInterface, typename EngineImplTraits::RenderDeviceImplType>
        {
        public:
            using TextureInterface = typename EngineImplTraits::TextureInterface;
            using RenderDeviceImplType = typename EngineImplTraits::RenderDeviceImplType;
            using TextureViewImplType = typename EngineImplTraits::TextureViewImplType;

            using TTextureBase = typename DeviceObjectBase<TextureInterface, RenderDeviceImplType>;

            Texture(RenderDeviceImplType* device) : TTextureBase(device) { }
            virtual ~Texture() = default;

            void create_from_file(const char* file_name);
            void create_from_memory(const void* data, uint32_t size);

            virtual void* get_native_texture() const = 0;

            TextureViewImplType** get_default_texture_views_array()
            {
                const uint32_t num_default_views = 1;
                return num_default_views > 1 ? 
                    reinterpret_cast<TextureViewImplType**>(m_pDefaultTextureViews) : 
                    reinterpret_cast<TextureViewImplType**>(&m_pDefaultTextureViews);
            }

            ITextureView* get_default_texture_view(TEXTURE_VIEW_USAGE view_type) const override
            {
                const uint32_t num_default_views = 1;
                return num_default_views > 1 ? 
                    reinterpret_cast<TextureViewImplType**>(m_pDefaultTextureViews)[static_cast<uint32_t>(view_type)] : 
                    reinterpret_cast<TextureViewImplType*>(m_pDefaultTextureViews);
            }

            virtual const TextureCreateDesc& get_create_desc() const override
            {
                return m_desc;
            }

        protected:
            virtual TextureViewImplType* create_view_internal(const TextureViewCreateDesc& desc) const = 0;

            void create_default_views()
            {
                cyber_assert(m_pDefaultTextureViews == nullptr, "Default texture views already created");

                const uint32_t num_default_views = 1;

                if(num_default_views == 0)
                    return;

                if(num_default_views > 1)
                {
                    m_pDefaultTextureViews = cyber_malloc(sizeof(TextureViewImplType*) * num_default_views);
                }

                auto** default_views = get_default_texture_views_array();

                uint32_t default_view_index = 0;

                auto CreateDefaultView = [&](TEXTURE_VIEW_USAGE Usage)
                {
                    TextureViewCreateDesc view_desc = {};
                    view_desc.m_pTexture = this;
                    view_desc.m_format = m_desc.m_format;
                    view_desc.m_usages = Usage;
                    view_desc.m_aspects = TEXTURE_VIEW_ASPECT::TVA_COLOR;
                    view_desc.m_dimension = TEXTURE_DIMENSION::TEX_DIMENSION_2D;
                    view_desc.m_baseArrayLayer = 0;
                    view_desc.m_arrayLayerCount = 1;
                    view_desc.m_baseMipLevel = 0;
                    view_desc.m_mipLevelCount = 1;

                    auto* view = create_view_internal(view_desc);
                    cyber_assert(view != nullptr, "Failed to create default texture view");
                    default_views[default_view_index++] = view;
                };

                if(m_desc.start_state & GRAPHICS_RESOURCE_STATE::GRAPHICS_RESOURCE_STATE_RENDER_TARGET)
                {
                    CreateDefaultView(TEXTURE_VIEW_USAGE::TVU_SRV);
                }
                
                if(m_desc.start_state & GRAPHICS_RESOURCE_STATE::GRAPHICS_RESOURCE_STATE_DEPTH_WRITE || m_desc.start_state & GRAPHICS_RESOURCE_STATE::GRAPHICS_RESOURCE_STATE_SHADER_RESOURCE)
                {
                    CreateDefaultView(TEXTURE_VIEW_USAGE::TVU_RTV_DSV);
                }

                if(m_desc.start_state & GRAPHICS_RESOURCE_STATE::GRAPHICS_RESOURCE_STATE_UNORDERED_ACCESS)
                {
                    CreateDefaultView(TEXTURE_VIEW_USAGE::TVU_UAV);
                }

                cyber_assert(default_view_index == num_default_views, "Not all default views were created");
            }
            
        protected:
            uint32_t m_width;
            uint32_t m_height;
            uint32_t m_depth;
            uint32_t m_arraySize;
            uint32_t m_mipLevels;
            uint32_t m_format;
            /// Flags specifying which aspects (COLOR,DEPTH,STENCIL) are included in the pVkImageView
            uint32_t m_aspectMask;
            uint32_t m_nodeIndex;
            uint32_t m_isCube;
            uint32_t m_isDedicated;
            uint32_t m_ownsImage;
            
            void* m_pNativeHandle;
            void* m_pDefaultTextureViews;

            TextureCreateDesc m_desc;
        };
    }
}
