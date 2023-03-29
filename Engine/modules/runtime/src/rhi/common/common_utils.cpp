#include "common_utils.h"
#include "EASTL/vector.h"
#include <stdint.h>
#include <string.h>

namespace Cyber 
{
    bool rhi_util_shader_resource_is_root_constant(const RHIShaderResource& resource, const RHIRootSignatureCreateDesc& desc)
    {
        
        return resource.type == RHI_SHADER_RESOURCE_TYPE_ROOT_CONSTANT;
    }

    void rhi_util_init_root_signature_tables(RHIRootSignature* rootSignature, const struct RHIRootSignatureCreateDesc& desc)
    {
        RHIShaderReflection* entery_reflection[32] = {0};
        // Pick shader reflection data
        for(uint32_t i = 0; i < desc.shader_count; ++i)
        {
            const RHIPipelineShaderCreateDesc& shader_desc = desc.shaders[i];
            // Find shader reflection data
            for(uint32_t j = 0; j < shader_desc.library->entry_count; ++j)
            {
                RHIShaderReflection& temp_entry_reflection = shader_desc.library->entry_reflections[j];
                if(strcmp(shader_desc.entry, temp_entry_reflection.entry_name) == 0)
                {
                    entery_reflection[i] = &temp_entry_reflection;
                    break;
                }
            }
            if(entery_reflection[i] == nullptr)
            {
                // If we didn't find the entry point, use the first one
                entery_reflection[i] = &shader_desc.library->entry_reflections[0];
            }
        }

        // Collect all resources
        rootSignature->pipeline_type = RHI_PIPELINE_TYPE_NONE;
        eastl::vector<RHIShaderResource> all_resources;
        eastl::vector<RHIShaderResource> all_push_constants;
        eastl::vector<RHIShaderResource> all_static_samplers;
        for(uint32_t i = 0; i < desc.shader_count; ++i)
        {

        }

    }
}