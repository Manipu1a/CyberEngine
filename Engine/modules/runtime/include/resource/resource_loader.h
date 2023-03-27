#pragma once

#include "rhi/rhi.h"
#include <stdint.h>
#include "renderer/renderer.h"

namespace Cyber
{
    namespace ResourceLoader
    {
        typedef enum EShaderStageLoadFlags
        {
            SHADER_STAGE_LOAD_FLAG_NONE = 0,
            SHADER_STAGE_LOAD_FLAG_ENABLE_PS_PRIMITIVEID = 1 << 0,
            SHADER_STAGE_LOAD_FLAG_ENABLE_VR_MULTIVIEW = 1 << 1,
        } ShaderStageLoadFlags;

        struct ShaderStageLoadDesc
        {
            const char* file_name;
            ERHIShaderStage stage;
            ShaderMacro* macros;
            uint32_t macro_count;
            const char* entry_point_name;
            EShaderStageLoadFlags flags;
        };

        struct ShaderLoadDesc
        {
            ShaderStageLoadDesc stages[RHI_SHADER_STAGE_COUNT];
            ERHIShaderTarget target;
            const ShaderConstant* pConstants;
            uint32_t constant_count;
        };

        ShaderLibraryRHIRef add_shader(Renderer& renderer, const ShaderLoadDesc& desc);
    }
}