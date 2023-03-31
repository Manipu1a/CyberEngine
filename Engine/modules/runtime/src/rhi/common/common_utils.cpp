#include "common_utils.h"
#include "EASTL/vector.h"
#include <stdint.h>
#include <string.h>

namespace Cyber 
{
    bool rhi_util_shader_resource_is_root_constant(const RHIShaderResource& resource, const RHIRootSignatureCreateDesc& desc)
    {
        if(resource.type == RHI_RESOURCE_TYPE_PUSH_CONTANT)
            return true;
        for(uint32_t i = 0;i < desc.push_constant_count; ++i)
        {
            if(strcmp(resource.name, desc.push_constant_names[i]) == 0)
            {
                return true;
            }
        }
        return false;
    }

    bool rhi_util_shader_resource_is_static_sampler(const RHIShaderResource& resource, const RHIRootSignatureCreateDesc& desc)
    {
        for(uint32_t i = 0; i < desc.static_sampler_count; ++i)
        {
            if(strcmp(resource.name, desc.static_sampler_names[i]) == 0)
            {
                return resource.type == RHI_RESOURCE_TYPE_SAMPLER;
            }
        }
        return false;
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
            RHIShaderReflection* reflection = entery_reflection[i];
            for(uint32_t j = 0; j < reflection->shader_resource_count; ++j)
            {
                // root constant and static sampler are unique
                RHIShaderResource& resource = reflection->shader_resources[j];
                if(rhi_util_shader_resource_is_root_constant(resource, desc))
                {
                    bool coincided = false;
                    for(auto& root_constant : all_push_constants)
                    {
                        if(root_constant.name_hash == resource.name_hash &&
                            root_constant.set == resource.set &&
                            root_constant.binding == resource.binding &&
                            root_constant.size == resource.size)
                        {
                            root_constant.stages |= resource.stages;
                            coincided = true;
                        }
                    }
                    if(!coincided)
                        all_push_constants.push_back(resource);
                }
                else if(rhi_util_shader_resource_is_static_sampler(resource, desc))
                {
                    bool coincided = false;
                    for(auto& static_sampler : all_static_samplers)
                    {
                        if(static_sampler.name_hash == resource.name_hash &&
                            static_sampler.set == resource.set &&
                            static_sampler.binding == resource.binding)
                        {
                            static_sampler.stages |= resource.stages;
                            coincided = true;
                        }
                    }
                    if(!coincided)
                        all_static_samplers.push_back(resource);
                }
                else
                {
                    all_resources.push_back(resource);
                }
            }

            // Merge pipeline type
            if(reflection->shader_stage & RHI_SHADER_STAGE_COMPUTE)
                rootSignature->pipeline_type = RHI_PIPELINE_TYPE_COMPUTE;
            else if(reflection->shader_stage & RHI_SHADER_STAGE_RAYTRACING)
                rootSignature->pipeline_type = RHI_PIPELINE_TYPE_RAYTRACING;
            else
                rootSignature->pipeline_type = RHI_PIPELINE_TYPE_GRAPHICS; 
        }

        // Merge root constants
        

    }
}