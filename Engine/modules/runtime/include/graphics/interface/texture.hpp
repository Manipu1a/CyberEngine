#pragma once
#include "common/cyber_graphics_config.h"
#include "common/flags.h"
#include "texture_view.h"
#include "device_object.h"
#include "interface/graphics_types.h"
#include "eastl/array.h"
#include "platform/memory.h"
#include "platform/platform_misc.h"
#include "EASTL/map.h"
#include "graphics/common/graphics_utils.hpp"
#include "core/common.h"
#include "log/Log.h"

namespace Cyber
{
    namespace RenderObject
    {
        struct CYBER_GRAPHICS_API TextureSubResData
        {
            // Pointer to the data in cpu memory
            const void *pData = nullptr;
            
            uint64_t srcOffset = 0;

            uint64_t stride = 0;

            uint64_t depthStride = 0;

            constexpr TextureSubResData() {}

            constexpr TextureSubResData(const void* data, uint64_t stride, uint64_t depthStride)
                : pData(data), srcOffset(0), stride(stride), depthStride(depthStride) {}

        };

        struct CYBER_GRAPHICS_API TextureData
        {
            TextureSubResData *pSubResources;

            uint32_t numSubResources;

            class IRenderDevice* pDevice;

            class ICommandBuffer* pCommandBuffer;
        };

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

            TEXTURE_DIMENSION m_dimension;
            GRAPHICS_RESOURCE_USAGE m_usage;
            uint32_t m_bindFlags;
            CPU_ACCESS_FLAGS m_cpuAccessFlags;
            TEXTURE_CREATE_FLAG m_flags;
            /// Number of multisamples per pixel (currently Textures created with mUsage TEXTURE_USAGE_SAMPLED_IMAGE only support SAMPLE_COUNT_1)
            TEXTURE_SAMPLE_COUNT m_sampleCount;
            /// The image quality level. The higher the quality, the lower the performance. The valid range is between zero and the value appropriate for mSampleCount
            uint32_t m_sampleQuality;
            /// Image format
            TEXTURE_FORMAT m_format;
            /// What state will the texture get created in
            GRAPHICS_RESOURCE_STATE m_initializeState;

            TextureCreateDesc()
                : m_pNativeHandle(nullptr), 
                  m_name(nullptr), 
                  m_width(0), 
                  m_height(0), 
                  m_depth(1), 
                  m_arraySize(1), 
                  m_mipLevels(1), 
                  m_dimension(TEXTURE_DIMENSION::TEX_DIMENSION_2D),
                  m_usage(GRAPHICS_RESOURCE_USAGE::GRAPHICS_RESOURCE_USAGE_DEFAULT),
                  m_bindFlags(GRAPHICS_RESOURCE_BIND_FLAGS::GRAPHICS_RESOURCE_BIND_SHADER_RESOURCE),
                  m_cpuAccessFlags(CPU_ACCESS_NONE),
                  m_flags(TCF_NONE),
                  m_sampleCount(TEXTURE_SAMPLE_COUNT::SAMPLE_COUNT_1),
                  m_sampleQuality(0),
                  m_format(TEX_FORMAT_UNKNOWN),
                  m_initializeState(GRAPHICS_RESOURCE_STATE::GRAPHICS_RESOURCE_STATE_COMMON)
            {
            }
        };

        CYBER_GRAPHICS_API MipLevelProperties compute_mip_level_properties(const RenderObject::TextureCreateDesc& load_info, uint32_t mip_level);
        struct CYBER_GRAPHICS_API ITexture : public IDeviceObject
        {
            virtual const TextureCreateDesc& get_create_desc() const = 0;
            virtual ITexture_View* get_default_texture_view(TEXTURE_VIEW_TYPE view_type) = 0;
            virtual GRAPHICS_RESOURCE_STATE get_old_state() const = 0;
            virtual void set_old_state(GRAPHICS_RESOURCE_STATE state) = 0;
            virtual GRAPHICS_RESOURCE_STATE get_new_state() const = 0;
            virtual void set_new_state(GRAPHICS_RESOURCE_STATE state) = 0;
        };

        template<typename EngineImplTraits>
        class CYBER_GRAPHICS_API Texture : public DeviceObjectBase<typename EngineImplTraits::TextureInterface, typename EngineImplTraits::RenderDeviceImplType>
        {
        public:
            using TextureInterface = typename EngineImplTraits::TextureInterface;
            using RenderDeviceImplType = typename EngineImplTraits::RenderDeviceImplType;
            using TextureViewImplType = typename EngineImplTraits::TextureViewImplType;

            using TTextureBase = DeviceObjectBase<TextureInterface, RenderDeviceImplType>;

            Texture(RenderDeviceImplType* device, TextureCreateDesc desc) : TTextureBase(device), m_desc(desc) 
            { 
                m_pNativeHandle = nullptr;
                m_pDefaultTextureViews = nullptr;

                if(m_desc.m_mipLevels == 0)
                {
                    if(m_desc.m_dimension == TEX_DIMENSION_1D || m_desc.m_dimension == TEX_DIMENSION_1D_ARRAY)
                    {
                        m_desc.m_mipLevels = compute_mip_levels_count(m_desc.m_width);
                    }
                    else if(m_desc.m_dimension == TEX_DIMENSION_2D || m_desc.m_dimension == TEX_DIMENSION_2D_ARRAY
                            || m_desc.m_dimension == TEX_DIMENSION_CUBE || m_desc.m_dimension == TEX_DIMENSION_CUBE_ARRAY)
                    {
                        m_desc.m_mipLevels = compute_mip_levels_count(m_desc.m_width, m_desc.m_height);
                    }
                    else if(m_desc.m_dimension == TEX_DIMENSION_3D)
                    {
                        m_desc.m_mipLevels = compute_mip_levels_count(m_desc.m_width, m_desc.m_height, m_desc.m_depth);
                    }
                    else
                    {
                        cyber_assert(false, "Unsupported texture dimension for mip level calculation");
                    }
                }

            }
            
            virtual ~Texture() = default;

            void create_from_file(const char* file_name);
            void create_from_memory(const void* data, uint32_t size);

            virtual void* get_native_texture() const = 0;

            uint32_t get_num_default_views() const
            {
                constexpr auto bind_flags_with_views  = 
                    GRAPHICS_RESOURCE_BIND_RENDER_TARGET | 
                    GRAPHICS_RESOURCE_BIND_DEPTH_STENCIL | 
                    GRAPHICS_RESOURCE_BIND_SHADER_RESOURCE | 
                    GRAPHICS_RESOURCE_BIND_UNORDERED_ACCESS | 
                    GRAPHICS_RESOURCE_BIND_SHADING_RATE;
                auto num_views = Platform_Misc::count_one_bits(
                    m_desc.m_bindFlags & bind_flags_with_views);
                return num_views;
            }

            TextureViewImplType** get_default_texture_views_array()
            {
                const uint32_t num_default_views = 1;
                return num_default_views > 1 ? 
                    reinterpret_cast<TextureViewImplType**>(m_pDefaultTextureViews) : 
                    reinterpret_cast<TextureViewImplType**>(&m_pDefaultTextureViews);
            }

            virtual ITexture_View* get_default_texture_view(TEXTURE_VIEW_TYPE view_type) override
            {
                auto view_index = views_indices_map[view_type];

                auto** default_views = get_default_texture_views_array();

                return reinterpret_cast<ITexture_View*>(default_views[view_index]);
            }

            virtual const TextureCreateDesc& get_create_desc() const override
            {
                return m_desc;
            }

            virtual GRAPHICS_RESOURCE_STATE get_old_state() const override
            {
                return m_oldState;
            }
            virtual void set_old_state(GRAPHICS_RESOURCE_STATE state) override
            {
                m_oldState = state;
            }
            virtual GRAPHICS_RESOURCE_STATE get_new_state() const override
            {
                return m_newState;
            }
            virtual void set_new_state(GRAPHICS_RESOURCE_STATE state) override
            {
                m_newState = state;
            }
            
        protected:
            virtual ITexture_View* create_view_internal(const TextureViewCreateDesc& desc) const = 0;

            void create_default_views()
            {
                cyber_assert(m_pDefaultTextureViews == nullptr, "Default texture views already created");

                const uint32_t num_default_views = get_num_default_views();

                if(num_default_views == 0)
                    return;

                if(num_default_views > 1)
                {
                    m_pDefaultTextureViews = cyber_malloc(sizeof(TextureViewImplType*) * num_default_views);
                }

                auto** default_views = get_default_texture_views_array();

                uint32_t default_view_index = 0;

                auto CreateDefaultView = [&](TEXTURE_VIEW_TYPE view_type)
                {
                    TextureViewCreateDesc view_desc = {};
                    view_desc.p_texture = this;
                    view_desc.format = m_desc.m_format;
                    view_desc.view_type = view_type;
                    view_desc.aspects = TEXTURE_VIEW_ASPECT::TVA_COLOR;
                    TEXTURE_DIMENSION dim = m_desc.m_dimension;
                    if((m_desc.m_dimension == TEX_DIMENSION_CUBE || m_desc.m_dimension == TEX_DIMENSION_CUBE_ARRAY) && view_type == TEXTURE_VIEW_TYPE::TEXTURE_VIEW_RENDER_TARGET)
                    {
                        dim = TEX_DIMENSION_2D_ARRAY;
                    }
                    view_desc.dimension = dim;
                    view_desc.baseArrayLayer = 0;
                    view_desc.arrayLayerCount = 1;
                    view_desc.baseMipLevel = 0;
                    // For shader resource views, include all mipmap levels
                    // For render target views, only include the first mipmap level
                    view_desc.mipLevelCount = (view_type == TEXTURE_VIEW_TYPE::TEXTURE_VIEW_SHADER_RESOURCE) ? m_desc.m_mipLevels : 1;
                    view_desc.p_native_resource = m_pNativeHandle;
                    auto* view = create_view_internal(view_desc);
                    cyber_assert(view != nullptr, "Failed to create default texture view");
                    default_views[default_view_index] = static_cast<TextureViewImplType*>(view);
                    views_indices_map[view_type] = default_view_index++;
                };

                if(m_desc.m_bindFlags & GRAPHICS_RESOURCE_BIND_RENDER_TARGET)
                {
                    CreateDefaultView(TEXTURE_VIEW_TYPE::TEXTURE_VIEW_RENDER_TARGET);
                }
                
                if(m_desc.m_bindFlags & GRAPHICS_RESOURCE_BIND_DEPTH_STENCIL)
                {
                    CreateDefaultView(TEXTURE_VIEW_TYPE::TEXTURE_VIEW_DEPTH_STENCIL);
                }

                if(m_desc.m_bindFlags & GRAPHICS_RESOURCE_BIND_SHADER_RESOURCE)
                {
                    CreateDefaultView(TEXTURE_VIEW_TYPE::TEXTURE_VIEW_SHADER_RESOURCE);
                }

                if(m_desc.m_bindFlags & GRAPHICS_RESOURCE_BIND_UNORDERED_ACCESS)
                {
                    CreateDefaultView(TEXTURE_VIEW_TYPE::TEXTURE_VIEW_UNORDERED_ACCESS);
                }

                if(m_desc.m_bindFlags & GRAPHICS_RESOURCE_BIND_SHADING_RATE)
                {
                    CreateDefaultView(TEXTURE_VIEW_TYPE::TEXTURE_VIEW_SHADING_RATE);
                }

                cyber_assert(default_view_index == num_default_views, "Not all default views were created");
            }
            
        protected:
            /// Flags specifying which aspects (COLOR,DEPTH,STENCIL) are included in the pVkImageView
            uint32_t m_aspectMask;
            uint32_t m_nodeIndex;
            uint32_t m_isCube;
            uint32_t m_isDedicated;
            uint32_t m_ownsImage;
            GRAPHICS_RESOURCE_STATE m_oldState;
            GRAPHICS_RESOURCE_STATE m_newState;
            void* m_pNativeHandle;
            void* m_pDefaultTextureViews;

            eastl::map<TEXTURE_VIEW_TYPE, uint8_t> views_indices_map;

            TextureCreateDesc m_desc;
        };
    }
}
