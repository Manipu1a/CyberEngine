#pragma once
#include "common/cyber_graphics_config.h"
#include "render_object.h"
#include "graphics_types.h"

namespace Cyber
{
    namespace RenderObject
    {
        class IShaderReflection;

        struct CYBER_GRAPHICS_API ShaderLibraryCreateDesc
        {
            const char8_t* name;
            const char8_t* entry_point;
            const void* code;
            uint32_t code_size;

            SHADER_STAGE stage;
            SHADER_COMPILER shader_compiler;
            SHADER_TARGET shader_target;
            uint32_t shader_macro_count;
            ShaderMacro* shader_macros;
        };

        struct CYBER_GRAPHICS_API IShaderLibrary
        {
            virtual const ShaderLibraryCreateDesc& get_create_desc() const = 0;
            virtual void free_reflection() = 0;
        };

        template<typename EngineImplTraits>
        class ShaderLibraryBase : public RenderObjectBase<typename EngineImplTraits::ShaderLibraryInterface, typename EngineImplTraits::RenderDeviceImplType>
        {
        public:
            using RenderDeviceImplType = typename EngineImplTraits::RenderDeviceImplType;

            ShaderLibraryBase(RenderDeviceImplType* device);
            virtual ~ShaderLibraryBase() = default;

            virtual const ShaderLibraryCreateDesc& get_create_desc() const
            {
                return create_desc;
            }
            char8_t* pName;
            IShaderReflection* entry_reflections;
            uint32_t entry_count;
            ShaderLibraryCreateDesc create_desc;
        };
    }

}