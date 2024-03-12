#pragma once
#include "common/cyber_graphics_config.h"
#include "render_object.h"
#include "graphics_types.h"

namespace Cyber
{
    namespace RenderObject
    {
        class IShaderResource;
        class IRootSignaturePool;
        
        struct CYBER_GRAPHICS_API RootSignatureParameterTable
        {
            // This should be stored here because shader could be destroyed after RS creation
            RenderObject::IShaderResource* resources = nullptr;
            uint32_t resource_count = 0;
            uint32_t set_index = 0;
        };

        struct CYBER_GRAPHICS_API RootSignatureCreateDesc
        {
            class PipelineShaderCreateDesc** shaders;
            uint32_t shader_count;
            ISampler** static_samplers;
            const char8_t* const* static_sampler_names;
            uint32_t static_sampler_count;
            const char8_t* const* push_constant_names;
            uint32_t push_constant_count;
            IRootSignaturePool* pool;
        };

        struct CYBER_GRAPHICS_API IRootSignature
        {
            
        };

        template<typename EngineImplTraits>
        class RootSignatureBase : public RenderObjectBase<typename EngineImplTraits::RootSignatureInterface, typename EngineImplTraits::RenderDeviceImplType>
        {
        public:
            using RootSignatureInterface = typename EngineImplTraits::RootSignatureInterface;
            using RenderDeviceImplType = typename EngineImplTraits::RenderDeviceImplType;
            using TRootSignatureBase = typename RootSignatureBase<RootSignatureInterface, RenderDeviceImplType>;

            RootSignatureBase(RenderDeviceImplType* device, const RootSignatureCreateDesc& desc) : TRootSignatureBase(device), m_desc(desc)
            {
                m_pParameterTables = nullptr;
                m_parameterTableCount = 0;
                m_pPushConstants = nullptr;
                m_pushConstantCount = 0;
                m_pStaticSamplers = nullptr;
                m_staticSamplerCount = 0;
                m_pPool = desc.pool;
                m_pipelineType = PIPELINE_TYPE_GRAPHICS;
                m_pPoolNext = nullptr;
            }

            virtual ~RootSignatureBase() = default;
        protected:
            RootSignatureParameterTable* m_pParameterTables;
            uint32_t m_parameterTableCount;
            class IShaderResource* m_pPushConstants;
            uint32_t m_pushConstantCount;
            class IShaderResource* m_pStaticSamplers;
            uint32_t m_staticSamplerCount;
            PIPELINE_TYPE m_pipelineType;
            class IRootSignaturePool* m_pPool;
            IRootSignature* m_pPoolNext;
            RootSignatureCreateDesc m_desc;
        };
    }

}