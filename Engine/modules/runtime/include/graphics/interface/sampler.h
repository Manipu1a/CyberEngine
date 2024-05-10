#pragma once
#include "graphics/interface/graphics_types.h"
#include "common/cyber_graphics_config.h"
#include "device_object.h"

namespace Cyber
{
    namespace RenderObject
    {
        struct CYBER_GRAPHICS_API SamplerCreateDesc
        {
            /// Texture minification filter
            FILTER_TYPE min_filter = FILTER_TYPE_LINEAR;
            /// Texture magnification filter
            FILTER_TYPE mag_filter = FILTER_TYPE_LINEAR;
            /// Texture mip-mapping filter
            FILTER_TYPE mip_filter = FILTER_TYPE_LINEAR;
            /// Texture address mode for U coordinate
            ADDRESS_MODE address_u = ADDRESS_MODE_CLAMP;
            /// Texture address mode for V coordinate
            ADDRESS_MODE address_v = ADDRESS_MODE_CLAMP;
            /// Texture address mode for W coordinate
            ADDRESS_MODE address_w = ADDRESS_MODE_CLAMP;

            SAMPLER_FLAG flags = SAMPLER_FLAG_NONE;

            bool unnormalized_coordinates = false;

            float mip_lod_bias = 0.0f;
            uint32_t max_anisotropy = 0;

            COMPARE_MODE compare_mode = CMP_NEVER;
        };
        struct CYBER_GRAPHICS_API ISampler : public IDeviceObject
        {
            
        };

        template<typename EngineImplTraits>
        class SamplerBase : public DeviceObjectBase<typename EngineImplTraits::SamplerInterface, typename EngineImplTraits::RenderDeviceImplType>
        {
        public:
            using SamplerInterface = typename EngineImplTraits::SamplerInterface;
            using RenderDeviceImplType = typename EngineImplTraits::RenderDeviceImplType;
            using TSamplerBase = typename DeviceObjectBase<SamplerInterface, RenderDeviceImplType>;

            SamplerBase(RenderDeviceImplType* device) : TSamplerBase(device) {  };
            virtual ~SamplerBase() = default;
        protected:
            
        };
    }

}