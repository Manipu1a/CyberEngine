#pragma once
#include "common/flags.h"
#include "texture_view.h"
#include "common/cyber_graphics_config.h"

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

        class CYBER_GRAPHICS_API Texture
        {
        public:
            Texture() = default;
            virtual ~Texture() = default;

            void create_from_file(const char* file_name);
            void create_from_memory(const void* data, uint32_t size);

            virtual void* get_native_texture() const = 0;
            virtual Texture_View* get_default_texture_view() const = 0;
            virtual Texture_View* create_texture_view(const TextureViewCreateDesc& desc) const = 0;

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
        };
    }
}
