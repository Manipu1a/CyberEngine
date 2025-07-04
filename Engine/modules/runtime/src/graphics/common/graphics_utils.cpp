#include "common/graphics_utils.hpp"
#include "EASTL/vector.h"
#include <stdint.h>
#include <string.h>
#include "containers/btree.h"
#include <EASTL/sort.h>
#include "interface/shader_resource.hpp"
#include "interface/render_pipeline.h"
#include "interface/shader_reflection.hpp"
#include "interface/shader_library.h"
#include "CyberLog/Log.h"
namespace Cyber 
{
    bool graphics_util_shader_resource_is_root_constant(const RenderObject::IShaderResource* resource, const RenderObject::RootSignatureCreateDesc& desc)
    {
        if(resource->get_type() == GRAPHICS_RESOURCE_TYPE::GRAPHICS_RESOURCE_TYPE_PUSH_CONTANT)
            return true;
        for(uint32_t i = 0;i < desc.m_pushConstantCount; ++i)
        {
            if(strcmp((char*)resource->get_name(), (char*)desc.m_pushConstantNames[i]) == 0)
            {
                return true;
            }
        }
        return false;
    }

    bool graphics_util_shader_resource_is_static_sampler(const RenderObject::IShaderResource* resource, const RenderObject::RootSignatureCreateDesc& desc)
    {
        return resource->get_type() == GRAPHICS_RESOURCE_TYPE_SAMPLER;
        
        for(uint32_t i = 0; i < desc.m_staticSamplerCount; ++i)
        {
            if(strcmp((char*)resource->get_name(), (char*)desc.m_staticSamplerNames[i]) == 0)
            {
                return resource->get_type() == GRAPHICS_RESOURCE_TYPE_SAMPLER;
            }
        }
        return false;
    }


    bool graphics_util_shader_resource_is_normal_sampler(const RenderObject::IShaderResource* resource)
    {
        return resource->get_type() == GRAPHICS_RESOURCE_TYPE_SAMPLER;
    }

    bool graphics_util_shader_resource_is_direct_cbv_srv_uav(const RenderObject::IShaderResource* resource, const RenderObject::RootSignatureCreateDesc& desc)
    {
        for(uint32_t i = 0; i < desc.root_descriptor_count; ++i)
        {
            if(strcmp((char*)resource->get_name(), (char*)desc.root_descriptor_names[i]) == 0)
            {
                return true;
            }
        }
        return false;
    }

    void graphics_util_init_root_signature_tables(struct RenderObject::IRootSignature* rootSignature, const struct RenderObject::RootSignatureCreateDesc& desc)
    {
        /*
        RenderObject::IShaderReflection* entery_reflection[32] = {0};
        // Pick shader reflection data
        for(uint32_t i = 0; i < desc.m_shaderCount; ++i)
        {
            const RenderObject::PipelineShaderCreateDesc* shader_desc = desc.m_ppShaders[i];
            // Find shader reflection data
            for(uint32_t j = 0; j < shader_desc->m_library->get_entry_count(); ++j)
            {
                RenderObject::IShaderReflection* temp_entry_reflection = shader_desc->m_library->get_entry_reflection(j);
                if(strcmp((char*)shader_desc->m_entry, (char*)temp_entry_reflection->get_entry_name()) == 0)
                {
                    entery_reflection[i] = temp_entry_reflection;
                    break;
                }
            }
            if(entery_reflection[i] == nullptr)
            {
                // If we didn't find the entry point, use the first one
                entery_reflection[i] = shader_desc->m_library->get_entry_reflection(0);
            }
        }

        // Collect all resources
        rootSignature->set_pipeline_type(PIPELINE_TYPE_NONE);
        eastl::vector<RenderObject::IShaderResource*> all_resources;
        eastl::vector<RenderObject::IShaderResource*> all_root_descriptors;
        eastl::vector<RenderObject::IShaderResource*> all_samplers;
        eastl::vector<RenderObject::IShaderResource*> all_push_constants;
        eastl::vector<RenderObject::IShaderResource*> all_static_samplers;

        for(uint32_t i = 0; i < desc.m_shaderCount; ++i)
        {
            RenderObject::IShaderReflection* reflection = entery_reflection[i];
            for(uint32_t j = 0; j < reflection->get_shader_resource_count(); ++j)
            {
                // root constant and static sampler are unique
                RenderObject::IShaderResource* resource = reflection->get_shader_resource(j);
                if(graphics_util_shader_resource_is_root_constant(resource, desc))
                {
                    bool coincided = false;
                    for(auto& root_constant : all_push_constants)
                    {
                        if(root_constant->get_name_hash() == resource->get_name_hash() &&
                            root_constant->get_set() == resource->get_set() &&
                            root_constant->get_binding() == resource->get_binding() &&
                            root_constant->get_size() == resource->get_size())
                        {
                            auto stages = root_constant->get_stages() | resource->get_stages();
                            root_constant->set_stages(stages);
                            coincided = true;
                        }
                    }
                    if(!coincided)
                        all_push_constants.push_back(resource);
                }
                else if(graphics_util_shader_resource_is_static_sampler(resource, desc))
                {
                    bool coincided = false;
                    for(auto& static_sampler : all_static_samplers)
                    {
                        if(static_sampler->get_name_hash() == resource->get_name_hash() &&
                            static_sampler->get_set() == resource->get_set() &&
                            static_sampler->get_binding() == resource->get_binding())
                        {
                            auto stages = static_sampler->get_stages() | resource->get_stages();
                            static_sampler->set_stages(stages);
                            coincided = true;
                        }
                    }
                    if(!coincided)
                        all_static_samplers.push_back(resource);
                }
                else if(graphics_util_shader_resource_is_direct_cbv_srv_uav(resource, desc))
                {
                    all_root_descriptors.push_back(resource);
                }
                else
                {
                    all_resources.push_back(resource);
                }
            }

            // Merge pipeline type
            if(reflection->get_shader_stage() & SHADER_STAGE_COMPUTE)
                rootSignature->set_pipeline_type(PIPELINE_TYPE_COMPUTE);
            else if(reflection->get_shader_stage() & SHADER_STAGE_RAYTRACING)
                rootSignature->set_pipeline_type(PIPELINE_TYPE_RAYTRACING);
            else
                rootSignature->set_pipeline_type(PIPELINE_TYPE_GRAPHICS);
        }

        // Merge root constants
        Cyber::btree_set<uint32_t> valid_sets;
        eastl::vector<RenderObject::IShaderResource*> rst_resources;
        rst_resources.reserve(all_resources.size());
        for(auto& shader_resource : all_resources)
        {
            bool coincided = false;
            for(auto& rst_resource : rst_resources)
            {
                if( rst_resource->get_set() == shader_resource->get_set() &&
                    rst_resource->get_binding() == shader_resource->get_binding() &&
                    rst_resource->get_type() == shader_resource->get_type())
                {
                    auto stages = rst_resource->get_stages();
                    stages |= shader_resource->get_stages();
                    rst_resource->set_stages(stages);
                    coincided = true;
                }
            }
            if(!coincided)
            {
                valid_sets.insert(shader_resource->get_set());
                rst_resources.emplace_back(shader_resource);
            }
        }
        eastl::stable_sort(rst_resources.begin(), rst_resources.end(), [](const RenderObject::IShaderResource* lhs, const RenderObject::IShaderResource* rhs){
            if(lhs->get_set() == rhs->get_set())
                return lhs->get_binding() < rhs->get_binding();
            return lhs->get_set() < rhs->get_set();
        });
        // Slice
        auto para_tables = (RenderObject::RootSignatureParameterTable**)cyber_calloc( (uint32_t)valid_sets.size(),sizeof(RenderObject::RootSignatureParameterTable*));
        uint32_t table_index = 0;
        for(auto&& set_index : valid_sets)
        {
            para_tables[table_index] = (RenderObject::RootSignatureParameterTable*)cyber_calloc(1, sizeof(RenderObject::RootSignatureParameterTable));
            RenderObject::RootSignatureParameterTable* table = para_tables[table_index];
            table->m_setIndex = set_index;
            table->m_resourceCount = 0;
            // 计算每个set的资源数量
            for(auto& rst_resource : rst_resources)
            {
                if(rst_resource->get_set() == set_index)
                    ++table->m_resourceCount;
            }
            table->m_ppResources = (RenderObject::IShaderResource**)cyber_calloc(table->m_resourceCount, sizeof(RenderObject::IShaderResource*));
            // 将参数按照set分组存入table
            uint32_t slot_index = 0;
            for(auto&& rst_resource : rst_resources)
            {
                if(rst_resource->get_set() == set_index)
                {
                    table->m_ppResources[slot_index] = rst_resource;
                    ++slot_index;
                }
            }
            ++table_index;
        }
        rootSignature->set_parameter_tables(para_tables, (uint32_t)valid_sets.size());
        // root descriptors
        uint32_t root_descriptor_count = (uint32_t)all_root_descriptors.size();
        rootSignature->set_root_descriptors((RenderObject::IShaderResource**)cyber_calloc(root_descriptor_count, sizeof(RenderObject::IShaderResource*)), root_descriptor_count);
        for(uint32_t i = 0; i < root_descriptor_count; ++i)
        {
            rootSignature->set_root_descriptor(all_root_descriptors[i], i);
        }
        // push constants
        uint32_t push_constant_count = (uint32_t)all_push_constants.size();
        rootSignature->set_push_constants((RenderObject::IShaderResource**)cyber_calloc(push_constant_count, sizeof(RenderObject::IShaderResource*)), push_constant_count);
        for(uint32_t i = 0; i < push_constant_count; ++i)
        {
            rootSignature->set_push_constant(all_push_constants[i], i);
        }
           
        // static samplers
        eastl::stable_sort(all_static_samplers.begin(), all_static_samplers.end(), [](const RenderObject::IShaderResource* lhs, const RenderObject::IShaderResource* rhs){
            if(lhs->get_set() == rhs->get_set())
                return lhs->get_binding() < rhs->get_binding();
            return lhs->get_set() < rhs->get_set();
        });
        uint32_t static_sampler_count = (uint32_t)all_static_samplers.size();
        rootSignature->set_static_samplers((RenderObject::IShaderResource**)cyber_calloc(static_sampler_count, sizeof(RenderObject::IShaderResource*)), static_sampler_count);
        for(uint32_t i = 0; i < static_sampler_count; ++i)
        {
            rootSignature->set_static_sampler(all_static_samplers[i], i);
        }
            */
    }

    void graphics_util_free_root_signature_tables(struct RenderObject::IRootSignature* rootSignature)
    {
        rootSignature->free();
    }

    eastl::string GetHLSLProfileString(SHADER_STAGE stage, ShaderVersion version)
    {
        eastl::string shader_profile;

        switch(stage)
        {
            case SHADER_STAGE_VERT: shader_profile = "vs"; break;
            case SHADER_STAGE_FRAG: shader_profile = "ps"; break;
            case SHADER_STAGE_GEOM: shader_profile = "gs"; break;
            case SHADER_STAGE_COMPUTE: shader_profile = "cs"; break;
            //case SHADER_STAGE_TESC: shader_profile = "hs"; break;
            //case SHADER_STAGE_DOMAIN: shader_profile = "ds"; break;
            case SHADER_STAGE_MESH: shader_profile = "ms"; break;
            case SHADER_STAGE_RAYTRACING: shader_profile = "lib"; break;
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

    CYBER_GRAPHICS_API uint32_t compute_mip_levels_count(uint32_t width)
    {
        if(width == 0)
            return 0;

        uint32_t levels = 0;
        while((width >> levels) > 0)
        {
            ++levels;
        }
        cyber_assert(width >= (1u << (levels - 1)) && width < (1u << levels), "Invalid mip levels count");
        return levels;
    }
    CYBER_GRAPHICS_API uint32_t compute_mip_levels_count(uint32_t width, uint32_t height)
    {
        return compute_mip_levels_count(std::max(width, height));
    }
    CYBER_GRAPHICS_API uint32_t compute_mip_levels_count(uint32_t width, uint32_t height, uint32_t depth)
    {
        return compute_mip_levels_count(std::max(std::max(width, height), depth));
    }

    CYBER_GRAPHICS_API const TextureFormatAttribs& get_texture_format_attribs(TEXTURE_FORMAT format)
    {
        static TextureFormatAttribs fmt_attribs[TEX_FORMAT_NUM_FORMATS];
        static bool is_init = false;

        if(!is_init)
        {
    #define INIT_TEX_FORMAT_INFO(TexFmt, ComponentSize, NumComponent, ComponentType, IsTypeless, BlockWidth, BlockHeight) \
            fmt_attribs[TexFmt] = TextureFormatAttribs( #TexFmt, TexFmt, ComponentSize, NumComponent, ComponentType, IsTypeless, BlockWidth, BlockHeight);

            INIT_TEX_FORMAT_INFO(TEX_FORMAT_RGBA32_TYPELESS,         4, 4, COMPONENT_TYPE_UNDEFINED,  true, 1,1);
            INIT_TEX_FORMAT_INFO(TEX_FORMAT_RGBA32_FLOAT,            4, 4, COMPONENT_TYPE_FLOAT,     false, 1,1);
            INIT_TEX_FORMAT_INFO(TEX_FORMAT_RGBA32_UINT,             4, 4, COMPONENT_TYPE_UINT,      false, 1,1);
            INIT_TEX_FORMAT_INFO(TEX_FORMAT_RGBA32_SINT,             4, 4, COMPONENT_TYPE_SINT,      false, 1,1);

            INIT_TEX_FORMAT_INFO(TEX_FORMAT_RGB32_TYPELESS,          4, 3, COMPONENT_TYPE_UNDEFINED,  true, 1,1);
            INIT_TEX_FORMAT_INFO(TEX_FORMAT_RGB32_FLOAT,             4, 3, COMPONENT_TYPE_FLOAT,     false, 1,1);
            INIT_TEX_FORMAT_INFO(TEX_FORMAT_RGB32_UINT,              4, 3, COMPONENT_TYPE_UINT,      false, 1,1);
            INIT_TEX_FORMAT_INFO(TEX_FORMAT_RGB32_SINT,              4, 3, COMPONENT_TYPE_SINT,      false, 1,1);

            INIT_TEX_FORMAT_INFO(TEX_FORMAT_RGBA16_TYPELESS,         2, 4, COMPONENT_TYPE_UNDEFINED,  true, 1,1);
            INIT_TEX_FORMAT_INFO(TEX_FORMAT_RGBA16_FLOAT,            2, 4, COMPONENT_TYPE_FLOAT,     false, 1,1);
            INIT_TEX_FORMAT_INFO(TEX_FORMAT_RGBA16_UNORM,            2, 4, COMPONENT_TYPE_UNORM,     false, 1,1);
            INIT_TEX_FORMAT_INFO(TEX_FORMAT_RGBA16_UINT,             2, 4, COMPONENT_TYPE_UINT,      false, 1,1);
            INIT_TEX_FORMAT_INFO(TEX_FORMAT_RGBA16_SNORM,            2, 4, COMPONENT_TYPE_SNORM,     false, 1,1);
            INIT_TEX_FORMAT_INFO(TEX_FORMAT_RGBA16_SINT,             2, 4, COMPONENT_TYPE_SINT,      false, 1,1);

            INIT_TEX_FORMAT_INFO(TEX_FORMAT_RG32_TYPELESS,           4, 2, COMPONENT_TYPE_UNDEFINED,  true, 1,1);
            INIT_TEX_FORMAT_INFO(TEX_FORMAT_RG32_FLOAT,              4, 2, COMPONENT_TYPE_FLOAT,     false, 1,1);
            INIT_TEX_FORMAT_INFO(TEX_FORMAT_RG32_UINT,               4, 2, COMPONENT_TYPE_UINT,      false, 1,1);
            INIT_TEX_FORMAT_INFO(TEX_FORMAT_RG32_SINT,               4, 2, COMPONENT_TYPE_SINT,      false, 1,1);

            INIT_TEX_FORMAT_INFO(TEX_FORMAT_R32G8X24_TYPELESS,       4, 2, COMPONENT_TYPE_DEPTH_STENCIL,  true, 1,1);
            INIT_TEX_FORMAT_INFO(TEX_FORMAT_D32_FLOAT_S8X24_UINT,    4, 2, COMPONENT_TYPE_DEPTH_STENCIL, false, 1,1);
            INIT_TEX_FORMAT_INFO(TEX_FORMAT_R32_FLOAT_X8X24_TYPELESS,4, 2, COMPONENT_TYPE_DEPTH_STENCIL, false, 1,1);
            INIT_TEX_FORMAT_INFO(TEX_FORMAT_X32_TYPELESS_G8X24_UINT, 4, 2, COMPONENT_TYPE_DEPTH_STENCIL, false, 1,1);

            INIT_TEX_FORMAT_INFO(TEX_FORMAT_RGB10A2_TYPELESS,        4, 1, COMPONENT_TYPE_COMPOUND,  true, 1,1);
            INIT_TEX_FORMAT_INFO(TEX_FORMAT_RGB10A2_UNORM,           4, 1, COMPONENT_TYPE_COMPOUND, false, 1,1);
            INIT_TEX_FORMAT_INFO(TEX_FORMAT_RGB10A2_UINT,            4, 1, COMPONENT_TYPE_COMPOUND, false, 1,1);
            INIT_TEX_FORMAT_INFO(TEX_FORMAT_R11G11B10_FLOAT,         4, 1, COMPONENT_TYPE_COMPOUND, false, 1,1);

            INIT_TEX_FORMAT_INFO(TEX_FORMAT_RGBA8_TYPELESS,          1, 4, COMPONENT_TYPE_UNDEFINED,   true, 1,1);
            INIT_TEX_FORMAT_INFO(TEX_FORMAT_RGBA8_UNORM,             1, 4, COMPONENT_TYPE_UNORM,      false, 1,1);
            INIT_TEX_FORMAT_INFO(TEX_FORMAT_RGBA8_UNORM_SRGB,        1, 4, COMPONENT_TYPE_UNORM_SRGB, false, 1,1);
            INIT_TEX_FORMAT_INFO(TEX_FORMAT_RGBA8_UINT,              1, 4, COMPONENT_TYPE_UINT,       false, 1,1);
            INIT_TEX_FORMAT_INFO(TEX_FORMAT_RGBA8_SNORM,             1, 4, COMPONENT_TYPE_SNORM,      false, 1,1);
            INIT_TEX_FORMAT_INFO(TEX_FORMAT_RGBA8_SINT,              1, 4, COMPONENT_TYPE_SINT,       false, 1,1);

            INIT_TEX_FORMAT_INFO(TEX_FORMAT_RG16_TYPELESS,           2, 2, COMPONENT_TYPE_UNDEFINED,  true, 1,1);
            INIT_TEX_FORMAT_INFO(TEX_FORMAT_RG16_FLOAT,              2, 2, COMPONENT_TYPE_FLOAT,     false, 1,1);
            INIT_TEX_FORMAT_INFO(TEX_FORMAT_RG16_UNORM,              2, 2, COMPONENT_TYPE_UNORM,     false, 1,1);
            INIT_TEX_FORMAT_INFO(TEX_FORMAT_RG16_UINT,               2, 2, COMPONENT_TYPE_UINT,      false, 1,1);
            INIT_TEX_FORMAT_INFO(TEX_FORMAT_RG16_SNORM,              2, 2, COMPONENT_TYPE_SNORM,     false, 1,1);
            INIT_TEX_FORMAT_INFO(TEX_FORMAT_RG16_SINT,               2, 2, COMPONENT_TYPE_SINT,      false, 1,1);

            INIT_TEX_FORMAT_INFO(TEX_FORMAT_R32_TYPELESS,            4, 1, COMPONENT_TYPE_UNDEFINED,  true, 1,1);
            INIT_TEX_FORMAT_INFO(TEX_FORMAT_D32_FLOAT,               4, 1, COMPONENT_TYPE_DEPTH,     false, 1,1);
            INIT_TEX_FORMAT_INFO(TEX_FORMAT_R32_FLOAT,               4, 1, COMPONENT_TYPE_FLOAT,     false, 1,1);
            INIT_TEX_FORMAT_INFO(TEX_FORMAT_R32_UINT,                4, 1, COMPONENT_TYPE_UINT,      false, 1,1);
            INIT_TEX_FORMAT_INFO(TEX_FORMAT_R32_SINT,                4, 1, COMPONENT_TYPE_SINT,      false, 1,1);

            INIT_TEX_FORMAT_INFO(TEX_FORMAT_R24G8_TYPELESS,          4, 1, COMPONENT_TYPE_DEPTH_STENCIL,  true, 1,1);
            INIT_TEX_FORMAT_INFO(TEX_FORMAT_D24_UNORM_S8_UINT,       4, 1, COMPONENT_TYPE_DEPTH_STENCIL, false, 1,1);
            INIT_TEX_FORMAT_INFO(TEX_FORMAT_R24_UNORM_X8_TYPELESS,   4, 1, COMPONENT_TYPE_DEPTH_STENCIL, false, 1,1);
            INIT_TEX_FORMAT_INFO(TEX_FORMAT_X24_TYPELESS_G8_UINT,    4, 1, COMPONENT_TYPE_DEPTH_STENCIL, false, 1,1);

            INIT_TEX_FORMAT_INFO(TEX_FORMAT_RG8_TYPELESS,            1, 2, COMPONENT_TYPE_UNDEFINED,  true, 1,1);
            INIT_TEX_FORMAT_INFO(TEX_FORMAT_RG8_UNORM,               1, 2, COMPONENT_TYPE_UNORM,     false, 1,1);
            INIT_TEX_FORMAT_INFO(TEX_FORMAT_RG8_UINT,                1, 2, COMPONENT_TYPE_UINT,      false, 1,1);
            INIT_TEX_FORMAT_INFO(TEX_FORMAT_RG8_SNORM,               1, 2, COMPONENT_TYPE_SNORM,     false, 1,1);
            INIT_TEX_FORMAT_INFO(TEX_FORMAT_RG8_SINT,                1, 2, COMPONENT_TYPE_SINT,      false, 1,1);

            INIT_TEX_FORMAT_INFO(TEX_FORMAT_R16_TYPELESS,            2, 1, COMPONENT_TYPE_UNDEFINED,  true, 1,1);
            INIT_TEX_FORMAT_INFO(TEX_FORMAT_R16_FLOAT,               2, 1, COMPONENT_TYPE_FLOAT,     false, 1,1);
            INIT_TEX_FORMAT_INFO(TEX_FORMAT_D16_UNORM,               2, 1, COMPONENT_TYPE_DEPTH,     false, 1,1);
            INIT_TEX_FORMAT_INFO(TEX_FORMAT_R16_UNORM,               2, 1, COMPONENT_TYPE_UNORM,     false, 1,1);
            INIT_TEX_FORMAT_INFO(TEX_FORMAT_R16_UINT,                2, 1, COMPONENT_TYPE_UINT,      false, 1,1);
            INIT_TEX_FORMAT_INFO(TEX_FORMAT_R16_SNORM,               2, 1, COMPONENT_TYPE_SNORM,     false, 1,1);
            INIT_TEX_FORMAT_INFO(TEX_FORMAT_R16_SINT,                2, 1, COMPONENT_TYPE_SINT,      false, 1,1);

            INIT_TEX_FORMAT_INFO(TEX_FORMAT_R8_TYPELESS,             1, 1, COMPONENT_TYPE_UNDEFINED,  true, 1,1);
            INIT_TEX_FORMAT_INFO(TEX_FORMAT_R8_UNORM,                1, 1, COMPONENT_TYPE_UNORM,     false, 1,1);
            INIT_TEX_FORMAT_INFO(TEX_FORMAT_R8_UINT,                 1, 1, COMPONENT_TYPE_UINT,      false, 1,1);
            INIT_TEX_FORMAT_INFO(TEX_FORMAT_R8_SNORM,                1, 1, COMPONENT_TYPE_SNORM,     false, 1,1);
            INIT_TEX_FORMAT_INFO(TEX_FORMAT_R8_SINT,                 1, 1, COMPONENT_TYPE_SINT,      false, 1,1);
            INIT_TEX_FORMAT_INFO(TEX_FORMAT_A8_UNORM,                1, 1, COMPONENT_TYPE_UNORM,     false, 1,1);

            INIT_TEX_FORMAT_INFO(TEX_FORMAT_R1_UNORM,                1, 1, COMPONENT_TYPE_UNORM,    false, 1,1);

            INIT_TEX_FORMAT_INFO(TEX_FORMAT_RGB9E5_SHAREDEXP,        4, 1, COMPONENT_TYPE_COMPOUND, false, 1,1);
            INIT_TEX_FORMAT_INFO(TEX_FORMAT_RG8_B8G8_UNORM,          1, 4, COMPONENT_TYPE_UNORM,    false, 1,1);
            INIT_TEX_FORMAT_INFO(TEX_FORMAT_G8R8_G8B8_UNORM,         1, 4, COMPONENT_TYPE_UNORM,    false, 1,1);

            // http://www.g-truc.net/post-0335.html
            // http://renderingpipeline.com/2012/07/texture-compression/
            INIT_TEX_FORMAT_INFO(TEX_FORMAT_BC1_TYPELESS,            8,  3, COMPONENT_TYPE_COMPRESSED,  true, 4,4);
            INIT_TEX_FORMAT_INFO(TEX_FORMAT_BC1_UNORM,               8,  3, COMPONENT_TYPE_COMPRESSED, false, 4,4);
            INIT_TEX_FORMAT_INFO(TEX_FORMAT_BC1_UNORM_SRGB,          8,  3, COMPONENT_TYPE_COMPRESSED, false, 4,4);
            INIT_TEX_FORMAT_INFO(TEX_FORMAT_BC2_TYPELESS,            16, 4, COMPONENT_TYPE_COMPRESSED,  true, 4,4);
            INIT_TEX_FORMAT_INFO(TEX_FORMAT_BC2_UNORM,               16, 4, COMPONENT_TYPE_COMPRESSED, false, 4,4);
            INIT_TEX_FORMAT_INFO(TEX_FORMAT_BC2_UNORM_SRGB,          16, 4, COMPONENT_TYPE_COMPRESSED, false, 4,4);
            INIT_TEX_FORMAT_INFO(TEX_FORMAT_BC3_TYPELESS,            16, 4, COMPONENT_TYPE_COMPRESSED,  true, 4,4);
            INIT_TEX_FORMAT_INFO(TEX_FORMAT_BC3_UNORM,               16, 4, COMPONENT_TYPE_COMPRESSED, false, 4,4);
            INIT_TEX_FORMAT_INFO(TEX_FORMAT_BC3_UNORM_SRGB,          16, 4, COMPONENT_TYPE_COMPRESSED, false, 4,4);
            INIT_TEX_FORMAT_INFO(TEX_FORMAT_BC4_TYPELESS,            8,  1, COMPONENT_TYPE_COMPRESSED,  true, 4,4);
            INIT_TEX_FORMAT_INFO(TEX_FORMAT_BC4_UNORM,               8,  1, COMPONENT_TYPE_COMPRESSED, false, 4,4);
            INIT_TEX_FORMAT_INFO(TEX_FORMAT_BC4_SNORM,               8,  1, COMPONENT_TYPE_COMPRESSED, false, 4,4);
            INIT_TEX_FORMAT_INFO(TEX_FORMAT_BC5_TYPELESS,            16, 2, COMPONENT_TYPE_COMPRESSED,  true, 4,4);
            INIT_TEX_FORMAT_INFO(TEX_FORMAT_BC5_UNORM,               16, 2, COMPONENT_TYPE_COMPRESSED, false, 4,4);
            INIT_TEX_FORMAT_INFO(TEX_FORMAT_BC5_SNORM,               16, 2, COMPONENT_TYPE_COMPRESSED, false, 4,4);

            INIT_TEX_FORMAT_INFO(TEX_FORMAT_B5G6R5_UNORM,            2, 1, COMPONENT_TYPE_COMPOUND, false, 1,1);
            INIT_TEX_FORMAT_INFO(TEX_FORMAT_B5G5R5A1_UNORM,          2, 1, COMPONENT_TYPE_COMPOUND, false, 1,1);
            INIT_TEX_FORMAT_INFO(TEX_FORMAT_BGRA8_UNORM,             1, 4, COMPONENT_TYPE_UNORM,    false, 1,1);
            INIT_TEX_FORMAT_INFO(TEX_FORMAT_BGRX8_UNORM,             1, 4, COMPONENT_TYPE_UNORM,    false, 1,1);
            INIT_TEX_FORMAT_INFO(TEX_FORMAT_R10G10B10_XR_BIAS_A2_UNORM,  4, 1, COMPONENT_TYPE_COMPOUND, false, 1,1);
            INIT_TEX_FORMAT_INFO(TEX_FORMAT_BGRA8_TYPELESS,          1, 4, COMPONENT_TYPE_UNDEFINED,     true, 1,1);
            INIT_TEX_FORMAT_INFO(TEX_FORMAT_BGRA8_UNORM_SRGB,        1, 4, COMPONENT_TYPE_UNORM_SRGB,   false, 1,1);
            INIT_TEX_FORMAT_INFO(TEX_FORMAT_BGRX8_TYPELESS,          1, 4, COMPONENT_TYPE_UNDEFINED,     true, 1,1);
            INIT_TEX_FORMAT_INFO(TEX_FORMAT_BGRX8_UNORM_SRGB,        1, 4, COMPONENT_TYPE_UNORM_SRGB,   false, 1,1);

            INIT_TEX_FORMAT_INFO(TEX_FORMAT_BC6H_TYPELESS,           16, 3, COMPONENT_TYPE_COMPRESSED,  true, 4,4);
            INIT_TEX_FORMAT_INFO(TEX_FORMAT_BC6H_UF16,               16, 3, COMPONENT_TYPE_COMPRESSED, false, 4,4);
            INIT_TEX_FORMAT_INFO(TEX_FORMAT_BC6H_SF16,               16, 3, COMPONENT_TYPE_COMPRESSED, false, 4,4);
            INIT_TEX_FORMAT_INFO(TEX_FORMAT_BC7_TYPELESS,            16, 4, COMPONENT_TYPE_COMPRESSED,  true, 4,4);
            INIT_TEX_FORMAT_INFO(TEX_FORMAT_BC7_UNORM,               16, 4, COMPONENT_TYPE_COMPRESSED, false, 4,4);
            INIT_TEX_FORMAT_INFO(TEX_FORMAT_BC7_UNORM_SRGB,          16, 4, COMPONENT_TYPE_COMPRESSED, false, 4,4);
    #undef  INIT_TEX_FORMAT_INFO

            is_init = true;
        }

        if(format >= TEX_FORMAT_UNKNOWN && format < TEX_FORMAT_NUM_FORMATS)
        {
            const auto& attribs = fmt_attribs[format];
            cyber_assert(attribs.format == format, "Invalid texture format");
            return attribs;
        }
        else
        {            
            return fmt_attribs[0];
        }
    }
}