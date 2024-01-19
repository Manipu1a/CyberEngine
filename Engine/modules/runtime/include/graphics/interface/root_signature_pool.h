#pragma once
#include "common/cyber_graphics_config.h"
#include "render_pass.h"

namespace Cyber
{
    namespace RenderObject
    {
        
        struct CYBER_GRAPHICS_API RootSignaturePoolCreateDesc
        {
            const char8_t* name;
        };

        struct CYBER_GRAPHICS_API IRootSignaturePool
        {
            virtual RootSignaturePoolCreateDesc get_create_desc() const = 0;
        };

        template<typename EngineImplTraits>
        class RootSignaturePoolBase : public RenderObjectBase<typename EngineImplTraits::RootSignaturePoolInterface, typename EngineImplTraits::RenderDeviceImplType>
        {
        public:
            using RenderDeviceImplType = typename EngineImplTraits::RenderDeviceImplType;

            RootSignaturePoolBase(RenderDeviceImplType* device);
            virtual ~RootSignaturePoolBase() = default;

            virtual RootSignaturePoolCreateDesc get_create_desc() const
            {
                return create_desc;
            }
        protected:
            ERHIPipelineType pipeline_type;
            RootSignaturePoolCreateDesc create_desc;
        };
    }

}