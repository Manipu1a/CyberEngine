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
            using RenderDeviceImplType = typename EngineImplTraits::RenderDeviceImplType;

            RootSignatureBase(RenderDeviceImplType* device);
            virtual ~RootSignatureBase() = default;
        protected:
            RootSignatureParameterTable* parameter_tables;
            uint32_t parameter_table_count;
            class IShaderResource* push_constants;
            uint32_t push_constant_count;
            class IShaderResource* static_samplers;
            uint32_t static_sampler_count;
            ERHIPipelineType pipeline_type;
            class IRootSignaturePool* pool;
            IRootSignature* pool_next;
        };
    }

}