#include "resource/resource_loader.h"
#include "EASTL/EABase/eabase.h"
#include "EASTL/string.h"
#include <corecrt_wstdio.h>
#include <stdint.h>
#include <stdio.h>
#include <vcruntime.h>
#include "CyberLog/Log.h"

namespace Cyber
{
    namespace ResourceLoader
    {
        FORCEINLINE bool load_shader_source_file(const char* fileName, char8_t** bytes, uint32_t* length)
        {
            FILE* f = fopen(fileName, "rb");
            fseek(f, 0, SEEK_END);
            *length = ftell(f);
            fseek(f, 0, SEEK_SET);
            *bytes = (char8_t*)cb_malloc(*length);
            fread(*bytes, *length, 1, f);
            fclose(f);
            return true;
        }

        FORCEINLINE bool load_shader_stage_byte_code(ERHIShaderTarget shaderTarget, const ShaderStageLoadDesc& loadDesc, uint32_t macroCount, ShaderMacro* macros, RHIShaderLibraryCreateDesc* libraryDesc, ShaderByteCodeBuffer* shaderByteCodeBuffer)
        {
            char8_t* bytes = nullptr;
            uint32_t length = 0;

            const char* rendererApi = "DX12";
            eastl::string fileNameAPI(eastl::string::CtorSprintf(), "%s/%s", rendererApi, loadDesc.file_name);

            CB_INFO("Compiling shader in runtime: [0] -> '[1]' macroCount=[2]", "DX12", loadDesc.entry_point_name, macroCount);
            load_shader_source_file(fileNameAPI.c_str(), &bytes, &length);
            libraryDesc->code = bytes;
            libraryDesc->code_size = length;
            libraryDesc->stage = loadDesc.stage;
            libraryDesc->name = "ShaderLibrary";
            eastl::string shaderDefines = "";

            // Apply user specified macros
            for(uint32_t i = 0;i < macroCount; ++i)
            {
                shaderDefines.append_sprintf("%s", macros[i].definition);
                shaderDefines.append_sprintf("%s", macros[i].value);
            }

            eastl::string binaryShaderComponent;
            static const size_t seed = 0x31415926;
            size_t shaderDefinesHash = rhi_hash(shaderDefines.c_str(), shaderDefines.size(), seed);
            binaryShaderComponent.append_sprintf("%s_%s_%zu_%s_%u", rendererApi, loadDesc.file_name, shaderDefinesHash, loadDesc.stage, shaderTarget);
            
            return true;
        }

        ShaderLibraryRHIRef add_shader(Renderer& renderer, const ShaderLoadDesc& desc)
        {
            RHIShaderLibraryCreateDesc libraryDesc;
            
            EShaderStageLoadFlags combinedFlags = SHADER_STAGE_LOAD_FLAG_NONE;

            ShaderByteCodeBuffer shaderByteCodeBuffer = {};
            {
                if(desc.stage_load_desc.file_name && desc.stage_load_desc.file_name[0] != '\0')
                {
                    const char* fileName = desc.stage_load_desc.file_name;

                    combinedFlags |= desc.stage_load_desc.flags;

                    uint32_t macroCount = desc.stage_load_desc.macro_count;
                    ShaderMacro* macros = (ShaderMacro*)cb_calloc(macroCount, sizeof(ShaderMacro));
                    for(uint32_t marcoIdx = 0; marcoIdx < macroCount; ++marcoIdx)
                    {
                        macros[marcoIdx] = desc.stage_load_desc.macros[marcoIdx];
                    }

                    load_shader_stage_byte_code(desc.target, desc.stage_load_desc, macroCount, macros, &libraryDesc, &shaderByteCodeBuffer);

                    ShaderLibraryRHIRef shaderLibrary = RHI::GetRHIContext().rhi_create_shader_library(renderer.GetDevice(), libraryDesc);
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