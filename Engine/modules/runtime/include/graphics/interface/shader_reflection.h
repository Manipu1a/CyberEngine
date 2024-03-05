#pragma once
#include "common/cyber_graphics_config.h"
#include "render_object.h"
#include "graphics_types.h"

namespace Cyber
{
    namespace RenderObject
    {
        class IShaderResource;
        class IVertexInput;
        
        struct CYBER_GRAPHICS_API IShaderReflection
        {
            
        };

        template<typename EngineImplTraits>
        class ShaderReflectionBase : public RenderObjectBase<typename EngineImplTraits::ShaderReflectionInterface, typename EngineImplTraits::RenderDeviceImplType>
        {
        public:
            using RenderDeviceImplType = typename EngineImplTraits::RenderDeviceImplType;

            ShaderReflectionBase(RenderDeviceImplType* device);
            virtual ~ShaderReflectionBase() = default;
            
            const char8_t* entry_name;
            char* name_pool;
            IVertexInput* vertex_inputs;
            RenderObject::IShaderResource* shader_resources;
            SHADER_STAGE shader_stage;
            uint32_t name_pool_size;
            uint32_t vertex_input_count;
            uint32_t shader_resource_count;
            uint32_t variable_count;

            // Thread group size for compute shader
            uint32_t num_threads_per_group;
            // number of tessellation control point
            uint32_t num_control_point;
            uint32_t thread_group_sizes[3];
        };
    }

}
