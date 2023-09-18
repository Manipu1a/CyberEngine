#pragma once

#include "rhi/rhi.h"
#include <stdint.h>
#include "renderer/renderer.h"

namespace Cyber
{
    namespace ResourceLoader
    {
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
            ERHIShaderStage stage;
            eastl::vector<ShaderMacro> macros;
            uint32_t macro_count;
            const char8_t* entry_point_name;
            EShaderStageLoadFlags flags;
        };

        struct ShaderLoadDesc
        {
            ShaderStageLoadDesc stage_load_desc;
            ERHIShaderTarget target;
            const ShaderConstant* pConstants;
            uint32_t constant_count;
        };

        RHIShaderLibrary* add_shader(Renderer& renderer, const ShaderLoadDesc& desc);
    }
}