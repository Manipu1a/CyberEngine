#pragma once
#include "common/cyber_graphics_config.h"
#include "device_object.h"
#include "graphics_types.h"
#include "platform/memory.h"

namespace Cyber
{
    namespace RenderObject
    {
        class IShaderResource;
        class IRootSignaturePool;
        
        struct CYBER_GRAPHICS_API RootSignatureParameterTable
        {
            // This should be stored here because shader could be destroyed after RS creation
            RenderObject::IShaderResource** resources = nullptr;
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
            virtual RootSignatureParameterTable* get_parameter_table(uint32_t index) const = 0;
            virtual uint32_t get_parameter_table_count() const = 0;
            virtual void set_parameter_tables(RootSignatureParameterTable** tables, uint32_t count) = 0;
            virtual void set_parameter_table(RootSignatureParameterTable* table, uint32_t index) = 0;
            virtual class IShaderResource** get_push_constants() const = 0;
            virtual class IShaderResource* get_push_constant(uint32_t index) const = 0;
            virtual uint32_t get_push_constant_count() const = 0;
            virtual void set_push_constants(class IShaderResource** pushConstants, uint32_t count) = 0;
            virtual void set_push_constant(class IShaderResource* pushConstant, uint32_t index) = 0;
            virtual class IShaderResource** get_static_samplers() const = 0;
            virtual class IShaderResource* get_static_sampler(uint32_t index) const = 0;
            virtual uint32_t get_static_sampler_count() const = 0;
            virtual void set_static_samplers(class IShaderResource** staticSamplers, uint32_t count) = 0;
            virtual void set_static_sampler(class IShaderResource* staticSampler, uint32_t index) = 0;
            virtual PIPELINE_TYPE get_pipeline_type() const = 0;
            virtual void set_pipeline_type(PIPELINE_TYPE type) = 0;
            virtual IRootSignaturePool* get_pool() const = 0;
            virtual IRootSignature* get_pool_next() const = 0;

            virtual RootSignatureCreateDesc get_desc() const = 0;
            virtual void free() = 0;
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

            virtual RootSignatureParameterTable* get_parameter_table(uint32_t index) const
            {
                return m_ppParameterTables[index];
            }

            virtual uint32_t get_parameter_table_count() const override
            {
                return m_parameterTableCount;
            }

            virtual void set_parameter_tables(RootSignatureParameterTable** tables, uint32_t count) override
            {
                m_ppParameterTables = tables;
                m_parameterTableCount = count;
            }
            
            virtual void set_parameter_table(RootSignatureParameterTable* table, uint32_t index) override
            {
                m_ppParameterTables[index] = table;
            }

            virtual class IShaderResource** get_push_constants() const override
            {
                return m_pPushConstants;
            }

            virtual class IShaderResource* get_push_constant(uint32_t index) const
            {
                return m_pPushConstants[index];
            }

            virtual uint32_t get_push_constant_count() const override
            {
                return m_pushConstantCount;
            }

            virtual void set_push_constants(class IShaderResource** pushConstants, uint32_t count)
            {
                m_pPushConstants = pushConstants;
                m_pushConstantCount = count;
            }

            virtual void set_push_constant(class IShaderResource* pushConstant, uint32_t index)
            {
                m_pPushConstants[index] = pushConstant;
            }

            virtual class IShaderResource** get_static_samplers() const override
            {
                return m_pStaticSamplers;
            }

            virtual uint32_t get_static_sampler_count() const override
            {
                return m_staticSamplerCount;
            }

            virtual class IShaderResource* get_static_sampler(uint32_t index) const
            {
                return m_pStaticSamplers[index];
            }
            virtual void set_static_samplers(class IShaderResource** staticSamplers, uint32_t count) override
            {
                m_pStaticSamplers = staticSamplers;
                m_staticSamplerCount = count;
            }
            virtual void set_static_sampler(class IShaderResource* staticSampler, uint32_t index) override
            {
                m_pStaticSamplers[index] = staticSampler;
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

            virtual void free() override
            {
                // free resources
                if(m_ppParameterTables)
                {
                    for(uint32_t i = 0; i < m_parameterTableCount; ++i)
                    {
                        RenderObject::RootSignatureParameterTable* table = m_ppParameterTables[i];
                        if(table->resources)
                        {
                            for(uint32_t binding = 0; binding < table->resource_count; ++binding)
                            {
                                RenderObject::IShaderResource* binding_to_free = table->resources[binding];
                                if(binding_to_free->get_name() != nullptr)
                                {
                                    binding_to_free->free();
                                }
                                cyber_free(binding_to_free);
                            }
                            cyber_free(table->resources);
                        }
                        cyber_free(table);
                    }
                    cyber_free(m_ppParameterTables);
                }
                // free constant
                if(m_pPushConstants)
                {
                    for(uint32_t i = 0;i < m_pushConstantCount; ++i)
                    {
                        RenderObject::IShaderResource* binding_to_free = m_pPushConstants[i];
                        if(binding_to_free.name != nullptr)
                        {
                            cyber_free((char8_t*)binding_to_free.name);
                            cyber_free(binding_to_free);
                        }
                    }
                    cyber_free(m_pPushConstants);
                }
                // free static samplers
                if(m_pStaticSamplers)
                {
                    for(uint32_t i = 0;i < m_staticSamplerCount; ++i)
                    {
                        RenderObject::IShaderResource* binding_to_free = m_pStaticSamplers[i];
                        if(binding_to_free.name != nullptr)
                        {
                            cyber_free((char8_t*)binding_to_free.name);
                        }
                    }
                    cyber_free(m_pStaticSamplers);
                }
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