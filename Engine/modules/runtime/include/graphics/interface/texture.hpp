#pragma once
#include "common/cyber_graphics_config.h"
#include "common/flags.h"
#include "texture_view.h"
#include "render_object.h"

namespace Cyber
{
    namespace RenderObject
    {
        struct CYBER_GRAPHICS_API TextureCreateDesc
        {
            /// Pointer to native texture handle if the texture does not own underlying resource
            void* native_handle;
            /// Debug name used in gpu profile
            const char8_t* name;
            uint32_t width;
            uint32_t height;
            uint32_t depth;
            uint32_t array_size;
            uint32_t mip_levels;
            /// Optimized clear value (recommended to use this same value when clearing the rendertarget)
            ERHIClearValue clear_value;
            ERHIDescriptorType descriptors;
            RHITextureCreationFlag flags;
            /// Number of multisamples per pixel (currently Textures created with mUsage TEXTURE_USAGE_SAMPLED_IMAGE only support SAMPLE_COUNT_1)
            ERHITextureSampleCount sample_count;
            /// The image quality level. The higher the quality, the lower the performance. The valid range is between zero and the value appropriate for mSampleCount
            uint32_t sample_quality;
            /// Image format
            ERHIFormat format;
            /// What state will the texture get created in
            ERHIResourceStates start_state;
        };


        struct CYBER_GRAPHICS_API ITexture
        {
            virtual ITextureView* get_default_texture_view(ERHITextureViewUsage view_type) const = 0;
            virtual const TextureCreateDesc& get_create_desc() const = 0;
        };

        template<typename EngineImplTraits>
        class CYBER_GRAPHICS_API Texture : public RenderObjectBase<typename EngineImplTraits::TextureInterface, typename EngineImplTraits::RenderDeviceImplType>
        {
        public:
            using TextureInterface = typename EngineImplTraits::TextureInterface;
            using RenderDeviceImplType = typename EngineImplTraits::RenderDeviceImplType;
            using TextureViewImplType = typename EngineImplTraits::TextureViewImplType;

            using TRenderObjectBase = RenderObjectBase<TextureInterface, RenderDeviceImplType>;

            Texture(RenderDeviceImplType* device) : TRenderObjectBase(device) { }
            virtual ~Texture() = default;

            void create_from_file(const char* file_name);
            void create_from_memory(const void* data, uint32_t size);

            virtual void* get_native_texture() const = 0;

            TextureViewImplType** get_default_texture_views_array()
            {
                const uint32_t num_default_views = 1;
                return num_default_views > 1 ? 
                    reinterpret_cast<TextureViewImplType**>(default_texture_views) : 
                    reinterpret_cast<TextureViewImplType**>(&default_texture_views);
            }

            ITextureView* get_default_texture_view(ERHITextureViewUsage view_type) const override
            {
                const uint32_t num_default_views = 1;
                return num_default_views > 1 ? 
                    reinterpret_cast<TextureViewImplType**>(default_texture_views)[static_cast<uint32_t>(view_type)] : 
                    reinterpret_cast<TextureViewImplType*>(default_texture_views);
            }

            virtual const TextureCreateDesc& get_create_desc() const override
            {
                return create_desc;
            }
            
            TextureCreateDesc create_desc;
        protected:
            virtual TextureViewImplType* create_view_internal(const TextureViewCreateDesc& desc) const = 0;
            
            void create_default_views()
            {
                cyber_assert(default_texture_views == nullptr, "Default texture views already created");

                const uint32_t num_default_views = 1;

                if(num_default_views == 0)
                    return;

                if(num_default_views > 1)
                {
                    default_texture_views = cyber_malloc(sizeof(TextureViewImplType*) * num_default_views);
                }

                auto** default_views = get_default_texture_views_array();

                uint32_t default_view_index = 0;

                auto CreateDefaultView = [&](ERHITextureViewUsage Usage)
                {
                    TextureViewCreateDesc view_desc = {};
                    view_desc.texture = this;
                    view_desc.format = create_desc.format;
                    view_desc.usages = Usage;
                    view_desc.aspects = ERHITextureViewAspect::RHI_TVA_COLOR;
                    view_desc.dimension = ERHITextureDimension::RHI_TEX_DIMENSION_2D;
                    view_desc.base_array_layer = 0;
                    view_desc.array_layer_count = 1;
                    view_desc.base_mip_level = 0;
                    view_desc.mip_level_count = 1;

                    auto* view = create_view_internal(view_desc);
                    cyber_assert(view != nullptr, "Failed to create default texture view");
                    default_views[default_view_index++] = view;
                };

                if(create_desc.start_state & ERHIResourceState::RHI_RESOURCE_STATE_RENDER_TARGET)
                {
                    CreateDefaultView(ERHITextureViewUsage::RHI_TVU_SRV);
                }
                
                if(create_desc.start_state & ERHIResourceState::RHI_RESOURCE_STATE_DEPTH_WRITE || create_desc.start_state & ERHIResourceState::RHI_RESOURCE_STATE_SHADER_RESOURCE)
                {
                    CreateDefaultView(ERHITextureViewUsage::RHI_TVU_RTV_DSV);
                }

                if(create_desc.start_state & ERHIResourceState::RHI_RESOURCE_STATE_UNORDERED_ACCESS)
                {
                    CreateDefaultView(ERHITextureViewUsage::RHI_TVU_UAV);
                }

                cyber_assert(default_view_index == num_default_views, "Not all default views were created");
            }
            
        protected:
            uint32_t mWidth;
            uint32_t mHeight;
            uint32_t mDepth;
            uint32_t mArraySize;
            uint32_t mMipLevels;
            uint32_t mFormat;
            /// Flags specifying which aspects (COLOR,DEPTH,STENCIL) are included in the pVkImageView
            uint32_t mAspectMask;
            uint32_t mNodeIndex;
            uint32_t mIsCube;
            uint32_t mIsDedicated;
            uint32_t mOwnsImage;
            
            void* mNativeHandle;
            void* default_texture_views;
        };
    }
}
