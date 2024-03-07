#include "common_utils.h"
#include "EASTL/vector.h"
#include <stdint.h>
#include <string.h>
#include "containers/btree.h"
#include <EASTL/sort.h>

namespace Cyber 
{
    bool graphics_util_shader_resource_is_root_constant(const IShaderResource& resource, const RootSignatureCreateDesc& desc)
    {
        if(resource.type == RHI_RESOURCE_TYPE_PUSH_CONTANT)
            return true;
        for(uint32_t i = 0;i < desc.push_constant_count; ++i)
        {
            if(strcmp((char*)resource.name, (char*)desc.push_constant_names[i]) == 0)
            {
                return true;
            }
        }
        return false;
    }

    bool graphics_util_shader_resource_is_static_sampler(const IShaderResource& resource, const RootSignatureCreateDesc& desc)
    {
        for(uint32_t i = 0; i < desc.static_sampler_count; ++i)
        {
            if(strcmp((char*)resource.name, (char*)desc.static_sampler_names[i]) == 0)
            {
                return resource.type == RHI_RESOURCE_TYPE_SAMPLER;
            }
        }
        return false;
    }
    
    void graphics_util_init_root_signature_tables(IRootSignature* rootSignature, const struct RootSignatureCreateDesc& desc)
    {
        RHIShaderReflection* entery_reflection[32] = {0};
        // Pick shader reflection data
        for(uint32_t i = 0; i < desc.shader_count; ++i)
        {
            const RHIPipelineShaderCreateDesc* shader_desc = desc.shaders[i];
            // Find shader reflection data
            for(uint32_t j = 0; j < shader_desc->library->entry_count; ++j)
            {
                RHIShaderReflection& temp_entry_reflection = shader_desc->library->entry_reflections[j];
                if(strcmp((char*)shader_desc->entry, (char*)temp_entry_reflection.entry_name) == 0)
                {
                    entery_reflection[i] = &temp_entry_reflection;
                    break;
                }
            }
            if(entery_reflection[i] == nullptr)
            {
                // If we didn't find the entry point, use the first one
                entery_reflection[i] = &shader_desc->library->entry_reflections[0];
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
        Cyber::btree_set<uint32_t> valid_sets;
        eastl::vector<RHIShaderResource> rst_resources;
        rst_resources.reserve(all_resources.size());
        for(auto& shader_resource : all_resources)
        {
            bool coincided = false;
            for(auto& rst_resource : rst_resources)
            {
                if( rst_resource.set == shader_resource.set &&
                    rst_resource.binding == shader_resource.binding &&
                    rst_resource.type == shader_resource.type)
                {
                    rst_resource.stages |= shader_resource.stages;
                    coincided = true;
                }
            }
            if(!coincided)
            {
                valid_sets.insert(shader_resource.set);
                rst_resources.emplace_back(shader_resource);
            }
        }
        eastl::stable_sort(rst_resources.begin(), rst_resources.end(), [](const RHIShaderResource& lhs, const RHIShaderResource& rhs){
            if(lhs.set == rhs.set)
                return lhs.binding < rhs.binding;
            return lhs.set < rhs.set;
        });
        // Slice
        rootSignature->parameter_table_count = (uint32_t)valid_sets.size();
        rootSignature->parameter_tables = (RHIParameterTable*)cyber_calloc( rootSignature->parameter_table_count,sizeof(RHIParameterTable));
        uint32_t table_index = 0;
        for(auto&& set_index : valid_sets)
        {
            RHIParameterTable& table = rootSignature->parameter_tables[table_index];
            table.set_index = set_index;
            table.resource_count = 0;
            // 计算每个set的资源数量
            for(auto& rst_resource : rst_resources)
            {
                if(rst_resource.set == set_index)
                    ++table.resource_count;
            }
            table.resources = (RHIShaderResource*)cyber_calloc(table.resource_count, sizeof(RHIShaderResource));
            // 将参数按照set分组存入table
            uint32_t slot_index = 0;
            for(auto&& rst_resource : rst_resources)
            {
                if(rst_resource.set == set_index)
                {
                    table.resources[slot_index] = rst_resource;
                    ++slot_index;
                }
            }
            ++table_index;
        }
        // push constants
        rootSignature->push_constant_count = (uint32_t)all_push_constants.size();
        rootSignature->push_constants = (RHIShaderResource*)cyber_calloc(rootSignature->push_constant_count, sizeof(RHIShaderResource));
        for(uint32_t i = 0; i < rootSignature->push_constant_count; ++i)
        {
            rootSignature->push_constants[i] = all_push_constants[i];
        }
        // static samplers
        eastl::stable_sort(all_static_samplers.begin(), all_static_samplers.end(), [](const RHIShaderResource& lhs, const RHIShaderResource& rhs){
            if(lhs.set == rhs.set)
                return lhs.binding < rhs.binding;
            return lhs.set < rhs.set;
        });
        rootSignature->static_sampler_count = (uint32_t)all_static_samplers.size();
        rootSignature->static_samplers = (RHIShaderResource*)cyber_calloc(rootSignature->static_sampler_count, sizeof(RHIShaderResource));
        for(uint32_t i = 0; i < rootSignature->static_sampler_count; ++i)
        {
            rootSignature->static_samplers[i] = all_static_samplers[i];
        }
    }

    void graphics_util_free_root_signature_tables(struct IRootSignature* rootSignature)
    {
        // free resources
        if(rootSignature->parameter_tables)
        {
            for(uint32_t i = 0; i < rootSignature->parameter_table_count; ++i)
            {
                RHIParameterTable& table = rootSignature->parameter_tables[i];
                if(table.resources)
                {
                    for(uint32_t binding = 0; binding < table.resource_count; ++binding)
                    {
                        RHIShaderResource& binding_to_free = table.resources[binding];
                        if(binding_to_free.name != nullptr)
                        {
                            cyber_free((char8_t*)binding_to_free.name);
                        }
                    }
                    cyber_free(table.resources);
                }
            }
            cyber_free(rootSignature->parameter_tables);
        }
        // free constant
        if(rootSignature->push_constants)
        {
            for(uint32_t i = 0;i < rootSignature->push_constant_count; ++i)
            {
                RHIShaderResource& binding_to_free = rootSignature->push_constants[i];
                if(binding_to_free.name != nullptr)
                {
                    cyber_free((char8_t*)binding_to_free.name);
                }
            }
            cyber_free(rootSignature->push_constants);
        }
        // free static samplers
        if(rootSignature->static_samplers)
        {
            for(uint32_t i = 0;i < rootSignature->static_sampler_count; ++i)
            {
                RHIShaderResource& binding_to_free = rootSignature->static_samplers[i];
                if(binding_to_free.name != nullptr)
                {
                    cyber_free((char8_t*)binding_to_free.name);
                }
            }
            cyber_free(rootSignature->static_samplers);
        }

    }

    eastl::string GetHLSLProfileString(SHADER_STAGE stage, ShaderVersion version)
    {
        eastl::string shader_profile;

        switch(stage)
        {
            case RHI_SHADER_STAGE_VERT: shader_profile = "vs"; break;
            case RHI_SHADER_STAGE_FRAG: shader_profile = "ps"; break;
            case RHI_SHADER_STAGE_GEOM: shader_profile = "gs"; break;
            case RHI_SHADER_STAGE_COMPUTE: shader_profile = "cs"; break;
            case RHI_SHADER_STAGE_TESC: shader_profile = "hs"; break;
            case RHI_SHADER_STAGE_DOMAIN: shader_profile = "ds"; break;
            case RHI_SHADER_STAGE_MESH: shader_profile = "ms"; break;
            case RHI_SHADER_STAGE_RAYTRACING: shader_profile = "lib"; break;
            default:
            {
                CB_WARN("Unknown shader type");
            }
        }
        
        shader_profile += "_";
        shader_profile += eastl::to_string(version.major);
        shader_profile += "_";
        shader_profile += eastl::to_string(version.minor);

        return shader_profile;
    }
}