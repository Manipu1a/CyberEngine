#pragma once
#include "common/cyber_graphics_config.h"
#include "shader_reflection.hpp"
#include "device_object.h"
#include "graphics_types.h"
#include "eastl/string.h"
#include "log/Log.h"

namespace Cyber
{
    namespace RenderObject
    {
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

        class CYBER_GRAPHICS_API IShaderLibrary : public IDeviceObject
        {
        public:
            virtual const ShaderLibraryCreateDesc& get_create_desc() const = 0;
            virtual void free_reflection() = 0;

            virtual IShaderReflection* get_entry_reflection(uint32_t index) = 0;
            virtual IShaderReflection** get_entry_reflections() = 0;
            virtual uint32_t get_entry_count() const = 0;

            virtual void get_material_resource_usage(MaterialResourceUsage& usage) const = 0;
        };
        /*
        typedef enum EShaderStageLoadFlag
        {
            SHADER_STAGE_LOAD_FLAG_NONE = 0,
            SHADER_STAGE_LOAD_FLAG_ENABLE_PS_PRIMITIVEID = 1 << 0,
            SHADER_STAGE_LOAD_FLAG_ENABLE_VR_MULTIVIEW = 1 << 1,
        } ShaderStageLoadFlag;
        typedef uint32_t EShaderStageLoadFlags;
        
        struct ShaderStageLoadDesc
        {
            const char8_t* file_name;
            SHADER_STAGE stage;
            eastl::vector<ShaderMacro> macros;
            uint32_t macro_count;
            const char8_t* entry_point_name;
            EShaderStageLoadFlags flags;
        };

        struct ShaderLoadDesc
        {
            ShaderStageLoadDesc stage_load_desc;
            SHADER_TARGET target;
            const ShaderConstant* pConstants;
            uint32_t constant_count;
        };

        CYBER_FORCE_INLINE bool load_shader_source_file(const char* fileName, char8_t** bytes, uint32_t* length)
        {
            FILE* f = fopen(fileName, "rb");
            fseek(f, 0, SEEK_END);
            *length = ftell(f);
            fseek(f, 0, SEEK_SET);
            *bytes = (char8_t*)cyber_malloc(*length);
            fread(*bytes, *length, 1, f);
            fclose(f);
            return true;
        }

        CYBER_FORCE_INLINE bool load_shader_stage_byte_code(SHADER_TARGET shaderTarget, const ShaderStageLoadDesc& loadDesc, uint32_t macroCount, ShaderMacro* macros, RenderObject::ShaderLibraryCreateDesc* libraryDesc, ShaderByteCodeBuffer* shaderByteCodeBuffer)
        {
            char8_t* bytes = nullptr;
            uint32_t length = 0;

            const char8_t* rendererApi = u8"../../../../shaders/DX12";
            eastl::string fileNameAPI(eastl::string::CtorSprintf(), "%s/%s", rendererApi, loadDesc.file_name);
            
            WCHAR assetsPath[512];
            DWORD size = GetModuleFileName(nullptr, assetsPath, sizeof(assetsPath));
            CB_CORE_INFO("Compiling shader in runtime: {0} -> '{1}' macroCount={2}", "DX12", (char*)loadDesc.entry_point_name, macroCount);
            load_shader_source_file(fileNameAPI.c_str(), &bytes, &length);
            libraryDesc->code = bytes;
            libraryDesc->code_size = length;
            libraryDesc->stage = loadDesc.stage;
            libraryDesc->name = loadDesc.file_name;
            libraryDesc->shader_compiler = SHADER_COMPILER_DEFAULT;
            libraryDesc->entry_point = loadDesc.entry_point_name;
            libraryDesc->shader_target = shaderTarget;
            libraryDesc->shader_macro_count = macroCount;
            libraryDesc->shader_macros = macros;
            eastl::string shaderDefines = "";

            // Apply user specified macros
            for(uint32_t i = 0;i < macroCount; ++i)
            {
                shaderDefines.append_sprintf("%s", macros[i].definition);
                shaderDefines.append_sprintf("%s", macros[i].value);
            }

            eastl::string binaryShaderComponent;
            static const size_t seed = 0x31415926;
            size_t shaderDefinesHash = graphics_hash(shaderDefines.c_str(), shaderDefines.size(), seed);
            binaryShaderComponent.append_sprintf("%s_%s_%zu_%u_%u", rendererApi, loadDesc.file_name, shaderDefinesHash, loadDesc.stage, shaderTarget);
            
            return true;
        }
        */

        template<typename EngineImplTraits>
        class ShaderLibraryBase : public DeviceObjectBase<typename EngineImplTraits::ShaderLibraryInterface, typename EngineImplTraits::RenderDeviceImplType>
        {
        public:
            using ShaderLibraryInterface = typename EngineImplTraits::ShaderLibraryInterface;
            using RenderDeviceImplType = typename EngineImplTraits::RenderDeviceImplType;
            using TShaderLibraryBase = DeviceObjectBase<ShaderLibraryInterface, RenderDeviceImplType>;

            ShaderLibraryBase(RenderDeviceImplType* device, const ShaderLibraryCreateDesc& desc) : TShaderLibraryBase(device), m_desc(desc)
            {
                m_name = desc.name;
                m_pEntryReflections = nullptr;
                m_entryCount = 1;

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

            virtual void get_material_resource_usage(MaterialResourceUsage& usage) const override
            {
                for (uint32_t i = 0; i < m_entryCount; ++i)
                {
                    if (m_pEntryReflections[i])
                    {
                        auto ref = m_pEntryReflections[i]->get_material_resource_usage();
                        usage.merge(ref);
                    }
                }
            }
        protected:
            const char8_t* m_name;
            IShaderReflection** m_pEntryReflections;
            uint32_t m_entryCount;
            const char8_t* m_entryPoint;
            ShaderLibraryCreateDesc m_desc;
            
        };
    }

}
