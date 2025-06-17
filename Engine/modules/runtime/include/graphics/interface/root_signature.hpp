#pragma once
#include "common/cyber_graphics_config.h"
#include "device_object.h"
#include "graphics_types.h"
#include "interface/shader_resource.hpp"
#include "platform/memory.h"

namespace Cyber
{
    namespace RenderObject
    {
        class IShaderResource;
        class IRootSignaturePool;
        class ISampler;
        class PipelineShaderCreateDesc;

        struct CYBER_GRAPHICS_API RootSignatureParameterTable
        {
            // This should be stored here because shader could be destroyed after RS creation
            RenderObject::IShaderResource** m_ppResources = nullptr;
            uint32_t m_resourceCount = 0;
            uint32_t m_setIndex = 0;
        };

        struct CYBER_GRAPHICS_API RootSignatureCreateDesc
        {
            RenderObject::PipelineShaderCreateDesc* vertex_shader = nullptr;
            RenderObject::PipelineShaderCreateDesc* pixel_shader = nullptr;
            RenderObject::PipelineShaderCreateDesc* mesh_shader = nullptr;
            RenderObject::PipelineShaderCreateDesc* geometry_shader = nullptr;

            RenderObject::ISampler** m_staticSamplers;
            const char8_t* const* m_staticSamplerNames;
            uint32_t m_staticSamplerCount;
            const char8_t* const* m_pushConstantNames;
            uint32_t m_pushConstantCount;
            const char8_t* const* root_descriptor_names;
            uint32_t root_descriptor_count;
            RenderObject::IRootSignaturePool** m_pPool;
        };

        struct CYBER_GRAPHICS_API IRootSignature : public IDeviceObject
        {
            virtual RootSignatureParameterTable** get_parameter_tables() const = 0;
            virtual RootSignatureParameterTable* get_parameter_table(uint32_t index) const = 0;
            virtual uint32_t get_parameter_table_count() const = 0;
            virtual void set_parameter_tables(RootSignatureParameterTable** tables, uint32_t count) = 0;
            virtual void set_parameter_table(RootSignatureParameterTable* table, uint32_t index) = 0;

            virtual class RenderObject::IShaderResource* get_root_descriptor(uint32_t index) const = 0;
            virtual class RenderObject::IShaderResource** get_root_descriptors() const = 0;
            virtual uint32_t get_root_descriptor_count() const = 0;
            virtual void set_root_descriptors(class RenderObject::IShaderResource** root_descriptors, uint32_t count) = 0;
            virtual void set_root_descriptor(class RenderObject::IShaderResource* root_descriptor, uint32_t index) = 0;

            virtual class RenderObject::IShaderResource** get_push_constants() const = 0;
            virtual class RenderObject::IShaderResource* get_push_constant(uint32_t index) const = 0;
            virtual uint32_t get_push_constant_count() const = 0;
            virtual void set_push_constants(class RenderObject::IShaderResource** pushConstants, uint32_t count) = 0;
            virtual void set_push_constant(class RenderObject::IShaderResource* pushConstant, uint32_t index) = 0;

            virtual class RenderObject::IShaderResource** get_static_samplers() const = 0;
            virtual class RenderObject::IShaderResource* get_static_sampler(uint32_t index) const = 0;
            virtual uint32_t get_static_sampler_count() const = 0;
            virtual void set_static_samplers(class RenderObject::IShaderResource** staticSamplers, uint32_t count) = 0;
            virtual void set_static_sampler(class RenderObject::IShaderResource* staticSampler, uint32_t index) = 0;
            virtual PIPELINE_TYPE get_pipeline_type() const = 0;
            virtual void set_pipeline_type(PIPELINE_TYPE type) = 0;
            virtual RenderObject::IRootSignaturePool** get_pool() const = 0;
            virtual void set_pool(RenderObject::IRootSignaturePool** pool) = 0;
            virtual RenderObject::IRootSignature* get_pool_next() const = 0;
            virtual void set_pool_next(RenderObject::IRootSignature* next) = 0;

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

            RootSignatureBase(RenderDeviceImplType* device, const RenderObject::RootSignatureCreateDesc& desc) : TRootSignatureBase(device), m_desc(desc)
            {
                m_ppParameterTables = nullptr;
                m_parameterTableCount = 0;
                m_pPushConstants = nullptr;
                m_pushConstantCount = 0;
                m_pStaticSamplers = nullptr;
                m_staticSamplerCount = 0;
                m_pPool = desc.m_pPool;
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

            virtual class RenderObject::IShaderResource* get_root_descriptor(uint32_t index) const override
            {
                return descriptor_resources[index];
            }
            virtual class RenderObject::IShaderResource** get_root_descriptors() const override
            {
                return descriptor_resources;
            }

            virtual uint32_t get_root_descriptor_count() const
            {
                return descriptor_resource_count;
            }

            virtual void set_root_descriptors(class RenderObject::IShaderResource** root_descriptors, uint32_t count)
            {
                descriptor_resources = root_descriptors;
                descriptor_resource_count = count;
            }

            virtual void set_root_descriptor(class RenderObject::IShaderResource* root_descriptor, uint32_t index) 
            {
                if(index < descriptor_resource_count)
                {
                    descriptor_resources[index] = root_descriptor;
                }
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
            
            virtual IRootSignaturePool** get_pool() const override
            {
                return m_pPool;
            }
            virtual void set_pool(RenderObject::IRootSignaturePool** pool) override
            {
                m_pPool = pool;
            }

            virtual IRootSignature* get_pool_next() const override
            {
                return m_pPoolNext;
            }
            virtual void set_pool_next(IRootSignature* next) override
            {
                m_pPoolNext = next;
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
                        if(table->m_ppResources)
                        {
                            for(uint32_t binding = 0; binding < table->m_resourceCount; ++binding)
                            {
                                RenderObject::IShaderResource* binding_to_free = table->m_ppResources[binding];
                                if(binding_to_free->get_name() != nullptr)
                                {
                                    binding_to_free->free();
                                }
                                cyber_free(binding_to_free);
                            }
                            cyber_free(table->m_ppResources);
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
                        binding_to_free->free();
                    }
                    cyber_free(m_pPushConstants);
                }
                // free static samplers
                if(m_pStaticSamplers)
                {
                    for(uint32_t i = 0;i < m_staticSamplerCount; ++i)
                    {
                        RenderObject::IShaderResource* binding_to_free = m_pStaticSamplers[i];
                        binding_to_free->free();
                    }
                    cyber_free(m_pStaticSamplers);
                }
            }

        protected:
            RootSignatureParameterTable** m_ppParameterTables;
            uint32_t m_parameterTableCount;
            class IShaderResource** descriptor_resources;
            uint32_t descriptor_resource_count;
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