#pragma once
#include "common/cyber_graphics_config.h"
#include "render_object.h"
#include "rhi.h"

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

            ERHIShaderStage stage;
            EShaderCompiler shader_compiler;
            ERHIShaderTarget shader_target;
            uint32_t shader_macro_count;
            ShaderMacro* shader_macros;
        };

        struct CYBER_GRAPHICS_API IShaderLibrary
        {
            virtual const ShaderLibraryCreateDesc& get_create_desc() const = 0;
        };

        template<typename EngineImplTraits>
        class ShaderLibraryBase : public RenderObjectBase<IShaderLibrary, typename EngineImplTraits::RenderDeviceImplType>
        {
        public:
            using RenderDeviceImplType = typename EngineImplTraits::RenderDeviceImplType;

            ShaderLibraryBase(RenderDeviceImplType* device);
            virtual ~ShaderLibraryBase() = default;

            virtual const ShaderLibraryCreateDesc& get_create_desc() const
            {
                return create_desc;
            }
        protected:
            char8_t* pName;
            IShaderReflection* entry_reflections;
            uint32_t entry_count;
            ShaderLibraryCreateDesc create_desc;
        };
    }

}
