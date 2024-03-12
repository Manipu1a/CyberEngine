#pragma once
#include "common/cyber_graphics_config.h"
#include "render_pass.h"

namespace Cyber
{
    namespace RenderObject
    {
        
        struct CYBER_GRAPHICS_API RootSignaturePoolCreateDesc
        {
            const char8_t* m_name;
        };

        struct CYBER_GRAPHICS_API IRootSignaturePool
        {
            virtual RootSignaturePoolCreateDesc get_create_desc() const = 0;
        };

        template<typename EngineImplTraits>
        class RootSignaturePoolBase : public RenderObjectBase<typename EngineImplTraits::RootSignaturePoolInterface, typename EngineImplTraits::RenderDeviceImplType>
        {
        public:
            using RootSignaturePoolInterface = typename EngineImplTraits::RootSignaturePoolInterface;
            using RenderDeviceImplType = typename EngineImplTraits::RenderDeviceImplType;
            using TRootSignaturePoolBase = typename RootSignaturePoolBase<RootSignaturePoolInterface, RenderDeviceImplType>;

            RootSignaturePoolBase(RenderDeviceImplType* device, const RootSignaturePoolCreateDesc& desc) : TRootSignaturePoolBase(device), m_desc(desc) {  };
            virtual ~RootSignaturePoolBase() = default;

            virtual RootSignaturePoolCreateDesc get_create_desc() const
            {
                return m_desc;
            }
        protected:
            PIPELINE_TYPE m_pipelineType;
            RootSignaturePoolCreateDesc m_desc;
        };
    }

}