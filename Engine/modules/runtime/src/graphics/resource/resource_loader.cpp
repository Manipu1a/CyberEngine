#include "resource/resource_loader.h"
#include "EASTL/EABase/eabase.h"
#include "EASTL/string.h"
#include <stdint.h>
#include <stdio.h>
#include <vcruntime.h>
#include "log/Log.h"
#include "platform/memory.h"
#include "interface/render_device.hpp"
#include "interface/shader_library.h"
#include "core/file_helper.hpp"

namespace Cyber
{
    namespace ResourceLoader
    {
        CYBER_FORCE_INLINE bool load_shader_source_file(const char* fileName, char8_t** bytes, uint32_t* length)
        {
            Core::FileHelper shader_file(fileName, Core::FILE_ACCESS_MODE::FILE_ACCESS_READ);
            IDataBlob* data_blob = nullptr;
            shader_file->read(&data_blob);
            *bytes = (char8_t*)data_blob->get_data_ptr();
            *length = data_blob->get_size();

            /*FILE* f = fopen(fileName, "rb");
            fseek(f, 0, SEEK_END);
            *length = ftell(f);
            fseek(f, 0, SEEK_SET);
            *bytes = (char8_t*)cyber_malloc(*length);
            fread(*bytes, *length, 1, f);
            fclose(f);*/

            return true;
        }

        CYBER_FORCE_INLINE bool load_shader_stage_byte_code(SHADER_TARGET shaderTarget, const ShaderStageLoadDesc& loadDesc, uint32_t macroCount, ShaderMacro* macros, RenderObject::ShaderLibraryCreateDesc* libraryDesc, ShaderByteCodeBuffer* shaderByteCodeBuffer)
        {
            char8_t* bytes = nullptr;
            uint32_t length = 0;

            const char8_t* rendererApi = u8"../../../../shaders/DX12";
            WCHAR assetsPath[512];
            DWORD size = GetModuleFileName(nullptr, assetsPath, sizeof(assetsPath));
            CB_CORE_INFO("Compiling shader in runtime: {0} -> '{1}' macroCount={2}", "DX12", (char*)loadDesc.entry_point_name, macroCount);
            load_shader_source_file((char*)loadDesc.file_name, &bytes, &length);
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

        CYBER_RUNTIME_API eastl::shared_ptr<RenderObject::IShaderLibrary> add_shader(RenderObject::IRenderDevice* device, const ShaderLoadDesc& desc)
        {
            RenderObject::ShaderLibraryCreateDesc libraryDesc;
            
            EShaderStageLoadFlags combinedFlags = SHADER_STAGE_LOAD_FLAG_NONE;

            ShaderByteCodeBuffer shaderByteCodeBuffer = {};
            {
                if(desc.stage_load_desc.file_name && desc.stage_load_desc.file_name[0] != '\0')
                {
                    const char8_t* fileName = desc.stage_load_desc.file_name;

                    combinedFlags |= desc.stage_load_desc.flags;

                    uint32_t macroCount = desc.stage_load_desc.macros.size();
                    ShaderMacro* macros = (ShaderMacro*)cyber_calloc(macroCount, sizeof(ShaderMacro));
                    for(uint32_t marcoIdx = 0; marcoIdx < macroCount; ++marcoIdx)
                    {
                        macros[marcoIdx] = desc.stage_load_desc.macros[marcoIdx];
                    }

                    load_shader_stage_byte_code(desc.target, desc.stage_load_desc, macroCount, macros, &libraryDesc, &shaderByteCodeBuffer);

                    eastl::shared_ptr<RenderObject::IShaderLibrary> shaderLibrary = device->create_shader_library(libraryDesc);
                    if(shaderLibrary)
                    {
                        return shaderLibrary;
                    }
                }
            }
            return nullptr;
        }
    }
}