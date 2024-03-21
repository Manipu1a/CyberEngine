#pragma once
#include "common/cyber_graphics_config.h"
#include "device_object.h"
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
            virtual RootSignatureParameterTable** get_parameter_tables() const = 0;
            virtual uint32_t get_parameter_table_count() const = 0;
            virtual class IShaderResource** get_push_constants() const = 0;
            virtual uint32_t get_push_constant_count() const = 0;
            virtual class IShaderResource** get_static_samplers() const = 0;
            virtual uint32_t get_static_sampler_count() const = 0;
            virtual PIPELINE_TYPE get_pipeline_type() const = 0;
            virtual void set_pipeline_type(PIPELINE_TYPE type) = 0;
            virtual IRootSignaturePool* get_pool() const = 0;
            virtual IRootSignature* get_pool_next() const = 0;

            virtual RootSignatureCreateDesc get_desc() const = 0;
        };

        template<typename EngineImplTraits>
        class RootSignatureBase : public DeviceObjectBase<typename EngineImplTraits::RootSignatureInterface, typename EngineImplTraits::RenderDeviceImplType>
        {
        public:
            using RootSignatureInterface = typename EngineImplTraits::RootSignatureInterface;
            using RenderDeviceImplType = typename EngineImplTraits::RenderDeviceImplType;
            using TRootSignatureBase = typename DeviceObjectBase<RootSignatureInterface, RenderDeviceImplType>;

            RootSignatureBase(RenderDeviceImplType* device, const RootSignatureCreateDesc& desc) : TRootSignatureBase(device), m_desc(desc)
            {
                m_ppParameterTables = nullptr;
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

            virtual RootSignatureParameterTable** get_parameter_tables() const override
            {
                return m_ppParameterTables;
            }

            virtual uint32_t get_parameter_table_count() const override
            {
                return m_parameterTableCount;
            }

            virtual class IShaderResource** get_push_constants() const override
            {
                return m_pPushConstants;
            }

            virtual uint32_t get_push_constant_count() const override
            {
                return m_pushConstantCount;
            }

            virtual class IShaderResource** get_static_samplers() const override
            {
                return m_pStaticSamplers;
            }

            virtual uint32_t get_static_sampler_count() const override
            {
                return m_staticSamplerCount;
            }

            virtual PIPELINE_TYPE get_pipeline_type() const override
            {
                return m_pipelineType;
            }

            virtual void set_pipeline_type(PIPELINE_TYPE type) override
            {
                m_pipelineType = type;
            }
            
            virtual IRootSignaturePool* get_pool() const override
            {
                return m_pPool;
            }

            virtual IRootSignature* get_pool_next() const override
            {
                return m_pPoolNext;
            }

            virtual RootSignatureCreateDesc get_desc() const override
            {
                return m_desc;
            }


        protected:
            RootSignatureParameterTable** m_ppParameterTables;
            uint32_t m_parameterTableCount;
            class IShaderResource** m_pPushConstants;
            uint32_t m_pushConstantCount;
            class IShaderResource** m_pStaticSamplers;
            uint32_t m_staticSamplerCount;
            PIPELINE_TYPE m_pipelineType;
            class IRootSignaturePool** m_pPool;
            IRootSignature* m_pPoolNext;
            RootSignatureCreateDesc m_desc;
        };
    }

}