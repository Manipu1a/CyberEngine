#pragma once
#include "common/cyber_graphics_config.h"
#include "device_object.h"
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

        struct CYBER_GRAPHICS_API IShaderLibrary : public IDeviceObject
        {
            virtual const ShaderLibraryCreateDesc& get_create_desc() const = 0;
            virtual void free_reflection() = 0;

            virtual IShaderReflection* get_entry_reflection(uint32_t index) = 0;
            virtual IShaderReflection** get_entry_reflections() = 0;
            virtual uint32_t get_entry_count() const = 0;
        };

        template<typename EngineImplTraits>
        class ShaderLibraryBase : public DeviceObjectBase<typename EngineImplTraits::ShaderLibraryInterface, typename EngineImplTraits::RenderDeviceImplType>
        {
        public:
            using ShaderLibraryInterface = typename EngineImplTraits::ShaderLibraryInterface;
            using RenderDeviceImplType = typename EngineImplTraits::RenderDeviceImplType;
            using TShaderLibraryBase = typename DeviceObjectBase<ShaderLibraryInterface, RenderDeviceImplType>;

            ShaderLibraryBase(RenderDeviceImplType* device, const ShaderLibraryCreateDesc& desc) : TShaderLibraryBase(device), m_desc(desc)
            {
                m_name = desc.name;
                m_pEntryReflections = nullptr;
                m_entryCount = 0;
            }

            virtual ~ShaderLibraryBase() = default;

            virtual const ShaderLibraryCreateDesc& get_create_desc() const
            {
                return m_desc;
            }

            virtual IShaderReflection* get_entry_reflection(uint32_t index)
            {
                return m_pEntryReflections[index];
            }

            virtual IShaderReflection** get_entry_reflections()
            {
                return m_pEntryReflections;
            }

            virtual uint32_t get_entry_count() const
            {
                return m_entryCount;
            }

        protected:
            const char8_t* m_name;
            IShaderReflection** m_pEntryReflections;
            uint32_t m_entryCount;
            ShaderLibraryCreateDesc m_desc;
        };
    }

}
