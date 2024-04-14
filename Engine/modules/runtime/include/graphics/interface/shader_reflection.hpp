#pragma once
#include "common/cyber_graphics_config.h"
#include "device_object.h"
#include "graphics_types.h"
#include "interface/vertex_input.h"
#include "interface/shader_resource.hpp"
#include "platform/memory.h"

namespace Cyber
{
    namespace RenderObject
    {
        class IShaderResource;
        class IVertexInput;
        
        struct CYBER_GRAPHICS_API IShaderReflection : public IDeviceObject
        {
            virtual const char8_t* get_entry_name() const = 0;
            virtual void set_entry_name(const char8_t* name) = 0;
            virtual const char* get_name_pool() const = 0;

            virtual IVertexInput** get_vertex_inputs() const = 0;
            virtual IVertexInput* get_vertex_input(uint32_t index) const = 0;
            virtual void set_vertex_inputs(IVertexInput** inputs) = 0;
            virtual void set_vertex_input(IVertexInput* input, uint32_t index) = 0;

            virtual void free_vertex_inputs() const = 0;

            virtual RenderObject::IShaderResource** get_shader_resources() const = 0;
            virtual RenderObject::IShaderResource* get_shader_resource(uint32_t index) const = 0;
            virtual void set_shader_resources(RenderObject::IShaderResource** resources) = 0;

            virtual void free_shader_resources() const = 0;

            virtual SHADER_STAGE get_shader_stage() const = 0;
            virtual void set_shader_stage(SHADER_STAGE stage) = 0;

            virtual uint32_t get_name_pool_size() const = 0;
            virtual uint32_t get_vertex_input_count() const = 0;
            virtual void set_vertex_input_count(uint32_t count) = 0;
            virtual uint32_t get_shader_resource_count() const = 0;
            virtual void set_shader_resource_count(uint32_t count) = 0;
            virtual uint32_t get_variable_count() const = 0;

            virtual uint32_t* get_thread_group_sizes() = 0;
            virtual void set_thread_group_sizes(uint32_t x, uint32_t y, uint32_t z) = 0;
        };

        template<typename EngineImplTraits>
        class ShaderReflectionBase : public DeviceObjectBase<typename EngineImplTraits::ShaderReflectionInterface, typename EngineImplTraits::RenderDeviceImplType>
        {
        public:
            using ShaderReflectionInterface = typename EngineImplTraits::ShaderReflectionInterface;
            using RenderDeviceImplType = typename EngineImplTraits::RenderDeviceImplType;
            using TShaderReflectionBase = typename DeviceObjectBase<ShaderReflectionInterface, RenderDeviceImplType>;

            ShaderReflectionBase(RenderDeviceImplType* device) : TShaderReflectionBase(device) {  };
            virtual ~ShaderReflectionBase() = default;
            
            virtual const char8_t* get_entry_name() const override final
            {
                return m_entryName;
            }

            virtual void set_entry_name(const char8_t* name) override final
            {
                m_entryName = name;
            }
            
            virtual const char* get_name_pool() const override final
            {
                return m_namePool;
            }

            virtual IVertexInput** get_vertex_inputs() const override final
            {
                return m_pVertexInputs;
            }

            virtual void set_vertex_inputs(IVertexInput** inputs) override final
            {
                m_pVertexInputs = inputs;
            }

            virtual void set_vertex_input(IVertexInput* input, uint32_t index) override final
            {
                m_pVertexInputs[index] = input;
            }
            
            virtual IVertexInput* get_vertex_input(uint32_t index) const final
            {
                return m_pVertexInputs[index];
            }

            virtual void free_vertex_inputs() const override final
            {
                if(m_pVertexInputs)
                {
                    for(uint32_t i = 0; i < m_vertexInputCount; i++)
                    {
                            m_pVertexInputs[i]->free();
                    }

                    cyber_free(m_pVertexInputs);
                }
            }

            virtual RenderObject::IShaderResource** get_shader_resources() const override final 
            {
                return m_ppShaderResources;
            }

            virtual RenderObject::IShaderResource* get_shader_resource(uint32_t index) const override final
            {
                return m_ppShaderResources[index];
            }
            
            virtual void set_shader_resources(RenderObject::IShaderResource** resources) override final
            {
                m_ppShaderResources = resources;
            }

            virtual void free_shader_resources() const override final
            {
                if(m_ppShaderResources)
                {
                    for(uint32_t i = 0; i < m_shaderResourceCount; i++)
                    {
                        m_ppShaderResources[i]->free();
                    }

                    cyber_free(m_ppShaderResources);
                }
            }
            
            virtual SHADER_STAGE get_shader_stage() const override final
            {
                return m_shaderStage;
            }

            virtual void set_shader_stage(SHADER_STAGE stage) override final
            {
                m_shaderStage = stage;
            }

            virtual uint32_t get_name_pool_size() const override final
            {
                return m_namePoolSize;
            }

            virtual uint32_t get_vertex_input_count() const override final
            {
                return m_vertexInputCount;
            }

            virtual void set_vertex_input_count(uint32_t count) override final
            {
                m_vertexInputCount = count;
            }

            virtual uint32_t get_shader_resource_count() const override final
            {
                return m_shaderResourceCount;
            }

            virtual void set_shader_resource_count(uint32_t count) override final
            {
                m_shaderResourceCount = count;
            }

            virtual uint32_t get_variable_count() const override final
            {
                return m_variableCount;
            }

            virtual uint32_t* get_thread_group_sizes() override final
            {
                return m_threadGroupSizes;
            }

            virtual void set_thread_group_sizes(uint32_t x, uint32_t y, uint32_t z) override final
            {
                m_threadGroupSizes[0] = x;
                m_threadGroupSizes[1] = y;
                m_threadGroupSizes[2] = z;
            }

        protected:
            const char8_t* m_entryName;
            char* m_namePool;
            IVertexInput** m_pVertexInputs;
            RenderObject::IShaderResource** m_ppShaderResources;
            SHADER_STAGE m_shaderStage;
            uint32_t m_namePoolSize;
            uint32_t m_vertexInputCount;
            uint32_t m_shaderResourceCount;
            uint32_t m_variableCount;

            // Thread group size for compute shader
            uint32_t m_numThreadsPerGroup;
            // number of tessellation control point
            uint32_t m_numControlPoint;
            uint32_t m_threadGroupSizes[3];
        };
    }

}
