#include "model_loader.h"
#include "log/Log.h"
#include "graphics/interface/graphics_types.h"
#include "texture_utils.h"

CYBER_BEGIN_NAMESPACE(Cyber)
CYBER_BEGIN_NAMESPACE(ModelLoader)

static std::string get_file_extension(const std::string& file_path)
{
    size_t last_dot = file_path.find_last_of('.');
    if (last_dot == std::string::npos || last_dot == file_path.length() - 1)
    {
        return "";
    }
    return file_path.substr(last_dot + 1);
}

struct GltfBufferAccessInfo
{
    const void* data = nullptr;
    size_t count = 0;
    int stride = 0;
    int component_type = 0;
    bool valid = false;
};

static GltfBufferAccessInfo get_accessor_data(const tinygltf::Model& gltf_model, int accessor_index)
{
    GltfBufferAccessInfo info;
    if (accessor_index < 0 || accessor_index >= static_cast<int>(gltf_model.accessors.size()))
        return info;

    const auto& accessor = gltf_model.accessors[accessor_index];
    info.count = accessor.count;
    info.component_type = accessor.componentType;

    if (accessor.bufferView < 0 || accessor.bufferView >= static_cast<int>(gltf_model.bufferViews.size()))
        return info;

    const auto& buffer_view = gltf_model.bufferViews[accessor.bufferView];
    if (buffer_view.buffer < 0 || buffer_view.buffer >= static_cast<int>(gltf_model.buffers.size()))
        return info;

    const auto& buffer = gltf_model.buffers[buffer_view.buffer];
    info.stride = accessor.ByteStride(buffer_view);
    info.data = &buffer.data[accessor.byteOffset + buffer_view.byteOffset];
    info.valid = true;
    return info;
}

Model::Model(const ModelCreateInfo& create_info)
{
    num_texture_attributes = static_cast<uint32_t>(DefaultTextureAttributes.size());
    texture_attribute_descs = DefaultTextureAttributes.data();
}

Model::Model(RenderObject::IRenderDevice* render_device, RenderObject::IDeviceContext* context, const ModelCreateInfo& create_info)
    : Model(create_info)
{
    load_from_file(render_device, context, create_info);
}

void Model::load_data(const ModelCreateInfo& create_info)
{
    if(create_info.file_path == nullptr)
    {
        cyber_assert(false, "Model file path is null.");
        return;
    }

    tinygltf::TinyGLTF gltf_context;

    std::string input_file_path = create_info.file_path;
    std::string file_extension = get_file_extension(input_file_path);
    m_base_dir = "";
    if(input_file_path.find_last_of("/\\") != std::string::npos)
    {
        m_base_dir = input_file_path.substr(0, input_file_path.find_last_of("/\\"));
    }
    m_base_dir += "/";

    std::string error;
    std::string warning;

    bool result = false;
    if(file_extension.compare("glb") == 0)
    {
        result = gltf_context.LoadBinaryFromFile(&model, &error, &warning, input_file_path.c_str());
    }
    else
    {
        result = gltf_context.LoadASCIIFromFile(&model, &error, &warning, input_file_path.c_str());
    }

    if(!result)
    {
        if(!warning.empty())
        {
            cyber_warn("GLTF Warning: {0}", warning.c_str());
        }

        if(!error.empty())
        {
            CB_ERROR("GLTF Error: {0}", error.c_str());
        }
        else
        {
            CB_ERROR("Failed to load GLTF model from file: {0}", input_file_path.c_str());
        }
        return;
    }

    for(auto& scene : model.scenes)
    {
        for(size_t i = 0; i < scene.nodes.size(); ++i)
        {
            load_node(model, static_cast<uint32_t>(scene.nodes[i]), float4x4::Identity());
        }
    }

    load_materials(model);
}

// Build the row-major local transform for a glTF node. Row-vector convention:
// world = local * parent, so local = Scale * Rotation * Translation.
static float4x4 gltf_node_local_transform(const tinygltf::Node& node)
{
    if (node.matrix.size() == 16)
    {
        return float4x4{
            (float)node.matrix[0],  (float)node.matrix[1],  (float)node.matrix[2],  (float)node.matrix[3],
            (float)node.matrix[4],  (float)node.matrix[5],  (float)node.matrix[6],  (float)node.matrix[7],
            (float)node.matrix[8],  (float)node.matrix[9],  (float)node.matrix[10], (float)node.matrix[11],
            (float)node.matrix[12], (float)node.matrix[13], (float)node.matrix[14], (float)node.matrix[15]
        };
    }

    float4x4 s = float4x4::Identity();
    if (node.scale.size() == 3)
        s = float4x4::scale((float)node.scale[0], (float)node.scale[1], (float)node.scale[2]);

    float4x4 r = float4x4::Identity();
    if (node.rotation.size() == 4)
    {
        Math::Quaternion<float> q((float)node.rotation[0], (float)node.rotation[1],
                                  (float)node.rotation[2], (float)node.rotation[3]);
        r = q.to_matrix();
    }

    float4x4 t = float4x4::Identity();
    if (node.translation.size() == 3)
        t = float4x4::translation((float)node.translation[0], (float)node.translation[1], (float)node.translation[2]);

    return s * r * t;
}

void Model::create_gpu_textures(RenderObject::IRenderDevice* render_device)
{
    load_textures(render_device, model, m_base_dir);
}

void Model::load_from_file(RenderObject::IRenderDevice* render_device, RenderObject::IDeviceContext* context, const ModelCreateInfo& create_info)
{
    load_data(create_info);
    create_gpu_textures(render_device);
}

BoundBox Model::compute_bounding_box(const float4x4& model_transform) const
{
    BoundBox model_bound_box;
    model_bound_box.Min = float3(FLT_MAX, FLT_MAX, FLT_MAX);
    model_bound_box.Max = float3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

    for(const auto& mesh : meshes)
    {
        const BoundBox& mesh_bound_box = mesh.bound_box.Transform(model_transform);
        model_bound_box.Min = Math::min(model_bound_box.Min, mesh_bound_box.Min);
        model_bound_box.Max = Math::max(model_bound_box.Max, mesh_bound_box.Max);
    }

    return model_bound_box;
}

void Model::load_node(const tinygltf::Model& gltf_model, uint32_t node_index, const float4x4& parent_transform)
{
    if(node_index >= gltf_model.nodes.size())
        return;

    const tinygltf::Node& node = gltf_model.nodes[node_index];
    const float4x4 world_transform = gltf_node_local_transform(node) * parent_transform;

    if (node.mesh >= 0 && node.mesh < static_cast<int>(gltf_model.meshes.size()))
    {
        load_mesh(gltf_model, node.mesh, world_transform);
    }

    for(const auto& child_id : node.children)
    {
        load_node(gltf_model, child_id, world_transform);
    }
}

void Model::load_mesh(const tinygltf::Model& gltf_model, uint32_t mesh_index, const float4x4& world_transform)
{
    if(mesh_index >= gltf_model.meshes.size())
        return;

    const tinygltf::Mesh& mesh = gltf_model.meshes[mesh_index];
    auto& new_mesh = meshes.emplace_back();

    for(size_t primitive_idx = 0; primitive_idx < mesh.primitives.size(); ++primitive_idx)
    {
        const tinygltf::Primitive& primitive = mesh.primitives[primitive_idx];

        uint32_t index_start = static_cast<uint32_t>(indices_data.size());
        uint32_t vertex_start = static_cast<uint32_t>(model_data.size());
        uint32_t index_count = 0;
        uint32_t vertex_count = 0;
        float3 pos_min(FLT_MAX, FLT_MAX, FLT_MAX);
        float3 pos_max(-FLT_MAX, -FLT_MAX, -FLT_MAX);

        // POSITION
        auto pos_it = primitive.attributes.find("POSITION");
        if (pos_it != primitive.attributes.end())
        {
            auto buf = get_accessor_data(gltf_model, pos_it->second);
            if (buf.valid && buf.component_type == TINYGLTF_COMPONENT_TYPE_FLOAT)
            {
                vertex_count = static_cast<uint32_t>(buf.count);
                model_data.resize(model_data.size() + vertex_count);

                const auto& accessor = gltf_model.accessors[pos_it->second];
                if (accessor.minValues.size() >= 3 && accessor.maxValues.size() >= 3)
                {
                    pos_min = float3((float)accessor.minValues[0], (float)accessor.minValues[1], (float)accessor.minValues[2]);
                    pos_max = float3((float)accessor.maxValues[0], (float)accessor.maxValues[1], (float)accessor.maxValues[2]);
                }

                const float* vertex_data = static_cast<const float*>(buf.data);
                auto component_count = buf.stride / sizeof(float);
                for (size_t j = 0; j < buf.count; ++j)
                {
                    size_t offset = j * component_count;
                    model_data[j + vertex_start].pos = { vertex_data[offset], vertex_data[offset + 1], vertex_data[offset + 2] };
                }
            }
        }

        // NORMAL
        auto normal_it = primitive.attributes.find("NORMAL");
        if (normal_it != primitive.attributes.end())
        {
            auto buf = get_accessor_data(gltf_model, normal_it->second);
            if (buf.valid && buf.component_type == TINYGLTF_COMPONENT_TYPE_FLOAT)
            {
                const float* normal_data = static_cast<const float*>(buf.data);
                auto component_count = buf.stride / sizeof(float);
                for (size_t j = 0; j < buf.count; ++j)
                {
                    size_t offset = j * component_count;
                    model_data[j + vertex_start].normal = { normal_data[offset], normal_data[offset + 1], normal_data[offset + 2] };
                }
            }
        }

        // TANGENT
        auto tangent_it = primitive.attributes.find("TANGENT");
        if (tangent_it != primitive.attributes.end())
        {
            auto buf = get_accessor_data(gltf_model, tangent_it->second);
            if (buf.valid && buf.component_type == TINYGLTF_COMPONENT_TYPE_FLOAT)
            {
                const float* tangent_data = static_cast<const float*>(buf.data);
                auto component_count = buf.stride / sizeof(float);
                for (size_t j = 0; j < buf.count; ++j)
                {
                    size_t offset = j * component_count;
                    model_data[j + vertex_start].tangent = { tangent_data[offset], tangent_data[offset + 1], tangent_data[offset + 2] };
                }
            }
        }

        // TEXCOORD_0
        auto uv_it = primitive.attributes.find("TEXCOORD_0");
        if (uv_it != primitive.attributes.end())
        {
            auto buf = get_accessor_data(gltf_model, uv_it->second);
            if (buf.valid && buf.component_type == TINYGLTF_COMPONENT_TYPE_FLOAT)
            {
                const float* uv_data = static_cast<const float*>(buf.data);
                auto component_count = buf.stride / sizeof(float);
                for (size_t j = 0; j < buf.count; ++j)
                {
                    size_t offset = j * component_count;
                    model_data[j + vertex_start].uv0 = { uv_data[offset], uv_data[offset + 1] };
                }
            }
        }

        // INDEX
        if(primitive.indices >= 0)
        {
            auto buf = get_accessor_data(gltf_model, primitive.indices);
            if (buf.valid)
            {
                index_count = static_cast<uint32_t>(buf.count);
                indices_data.resize(indices_data.size() + index_count);

                switch (buf.component_type)
                {
                    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
                    {
                        const uint8_t* index_data = static_cast<const uint8_t*>(buf.data);
                        for (size_t j = 0; j < index_count; ++j)
                            indices_data[index_start + j] = static_cast<uint32_t>(index_data[j]) + vertex_start;
                        break;
                    }
                    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
                    {
                        const uint16_t* index_data = static_cast<const uint16_t*>(buf.data);
                        for (size_t j = 0; j < index_count; ++j)
                            indices_data[index_start + j] = static_cast<uint32_t>(index_data[j]) + vertex_start;
                        break;
                    }
                    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
                    {
                        const uint32_t* index_data = static_cast<const uint32_t*>(buf.data);
                        for (size_t j = 0; j < index_count; ++j)
                            indices_data[index_start + j] = index_data[j] + vertex_start;
                        break;
                    }
                    default:
                        cyber_assert(false, "Unsupported index component type");
                        break;
                }
            }
        }

        // Bake the node's accumulated world transform into the vertex data.
        // Row-vector convention: p' = [p 1] * M.
        const bool is_identity = (world_transform == float4x4::Identity());
        if (!is_identity && vertex_count > 0)
        {
            const float4x4& M = world_transform;
            auto mul_point = [&](const float3& p) {
                return float3(
                    p.x * M.m00 + p.y * M.m10 + p.z * M.m20 + M.m30,
                    p.x * M.m01 + p.y * M.m11 + p.z * M.m21 + M.m31,
                    p.x * M.m02 + p.y * M.m12 + p.z * M.m22 + M.m32);
            };
            auto mul_dir = [&](const float3& d) {
                return float3(
                    d.x * M.m00 + d.y * M.m10 + d.z * M.m20,
                    d.x * M.m01 + d.y * M.m11 + d.z * M.m21,
                    d.x * M.m02 + d.y * M.m12 + d.z * M.m22);
            };

            pos_min = float3(FLT_MAX, FLT_MAX, FLT_MAX);
            pos_max = float3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
            for (uint32_t j = 0; j < vertex_count; ++j)
            {
                auto& v = model_data[vertex_start + j];

                v.pos = mul_point(v.pos);
                pos_min = Math::min(pos_min, v.pos);
                pos_max = Math::max(pos_max, v.pos);

                v.normal  = normalize(mul_dir(v.normal));
                v.tangent = mul_dir(v.tangent);
            }
        }

        new_mesh.primitives.emplace_back(Primitive{  index_start,
                                                    index_count,
                                                    vertex_count,
                                                    primitive.material >= 0 ? (uint32_t)primitive.material : 0 , pos_min, pos_max});
    }

    new_mesh.update_bounding_box();
}

void Model::load_materials(const tinygltf::Model& gltf_model)
{
    materials.reserve(gltf_model.materials.size());
    for (const auto& gltf_material : gltf_model.materials)
    {
        Material material;

        auto Find_Texture = [&material](const TextureAttributeDesc& attribs, const auto& mapping) {
            auto tex_it = mapping.find(attribs.name);
            if(tex_it == mapping.end())
                return false;

            material.texture_ids[attribs.index] = tex_it->second.TextureIndex();
            (&material.attribs.uv_selector0)[attribs.index] = tex_it->second.TextureTexCoord();

            return true;
        };

        for(size_t i = 0; i < num_texture_attributes; ++i)
        {
            const auto& texture_attrib = texture_attribute_descs[i];
            auto found = Find_Texture(texture_attrib, gltf_material.values);

            if(!found)
            {
                found = Find_Texture(texture_attrib, gltf_material.additionalValues);
            }
        }

        auto Read_Factor = [](float& factor, const tinygltf::ParameterMap& params, const char* name)
        {
            auto it = params.find(name);
            if (it != params.end())
            {
                factor = static_cast<float>(it->second.Factor());
            }
        };

        Read_Factor(material.attribs.roughness_factor, gltf_material.values, "roughnessFactor");
        Read_Factor(material.attribs.metallic_factor, gltf_material.values, "metallicFactor");

        auto Read_Color_Factor = [](float4& factor, const tinygltf::ParameterMap& params, const char* name)
        {
            auto it = params.find(name);
            if (it != params.end())
            {
                auto color_factors = it->second.ColorFactor();
                factor = { static_cast<float>(color_factors[0]), static_cast<float>(color_factors[1]), static_cast<float>(color_factors[2]), static_cast<float>(color_factors[3]) };
            }
        };

        Read_Color_Factor(material.attribs.base_color_factor, gltf_material.values, "baseColorFactor");
        Read_Color_Factor(material.attribs.emissive_factor, gltf_material.values, "emissiveFactor");

        {
            auto alpha_mode_it = gltf_material.values.find("alphaMode");
            if (alpha_mode_it != gltf_material.values.end())
            {
                const tinygltf::Parameter& param = alpha_mode_it->second;
                if(param.string_value == "BLEND")
                {
                    material.attribs.alpha_mode = Material::ALPHA_MODE_BLEND;
                }
                if(param.string_value == "MASK")
                {
                    material.attribs.alpha_mode = Material::ALPHA_MODE_MASK;
                    material.attribs.alpha_cutoff = param.number_value;
                }
            }
        }
        Read_Factor(material.attribs.alpha_cutoff, gltf_material.values, "alphaCutoff");

        {
            auto double_sided_it = gltf_material.values.find("doubleSided");
            if (double_sided_it != gltf_material.values.end())
            {
                material.double_sided = double_sided_it->second.bool_value;
            }
        }
        material.attribs.work_flow = Material::PBR_WORKFLOW_METALL_ROUGH;

        materials.push_back(material);
    }

    if(materials.empty())
    {
        materials.push_back(Material());
    }
}

void Model::load_textures(RenderObject::IRenderDevice* render_device, const tinygltf::Model& gltf_model, const std::string& base_dir)
{
    textures.reserve(gltf_model.textures.size());
    for (const auto& gltf_texture : gltf_model.textures)
    {
        if (gltf_texture.source < 0 || gltf_texture.source >= static_cast<int>(gltf_model.images.size()))
        {
            cyber_warn("GLTF Warning: Texture source index {0} is out of bounds", gltf_texture.source);
            continue;
        }

        const auto& gltf_image = gltf_model.images[gltf_texture.source];

        ImageData image_data;
        image_data.width = gltf_image.width;
        image_data.height = gltf_image.height;
        image_data.num_components = gltf_image.component;
        image_data.component_size = gltf_image.bits / 8;
        image_data.file_format = (gltf_image.width <= 0 || gltf_image.height <= 0) ? static_cast<TextureLoader::IMAGE_FILE_FORMAT>(gltf_image.pixel_type) : TextureLoader::IMAGE_FILE_FORMAT::IMAGE_FILE_FORMAT_UNKNOWN;
        image_data.pData = gltf_image.image.data();
        image_data.data_size = gltf_image.image.size();
        image_data.name = reinterpret_cast<const char8_t*>(gltf_image.uri.c_str());

        add_texture(render_device, image_data);
    }
}

TEXTURE_FORMAT get_image_data_texture_format(const Model::ImageData& image_data)
{
    if(image_data.tex_format != TEXTURE_FORMAT::TEX_FORMAT_UNKNOWN)
    {
        return image_data.tex_format;
    }

    cyber_check_msg(image_data.component_size == 1, "only 8-bit components are supported");

    switch (image_data.num_components)
    {
        case 1: return TEXTURE_FORMAT::TEX_FORMAT_R8_UNORM;
        case 2: return TEXTURE_FORMAT::TEX_FORMAT_RG8_UNORM;
        case 3:
        case 4: return TEXTURE_FORMAT::TEX_FORMAT_RGBA8_UNORM;
        default:
            cyber_assert(false, "Unsupported number of components");
            return TEXTURE_FORMAT::TEX_FORMAT_UNKNOWN;
    }
}

RenderObject::TextureData prepare_gltf_texture_data(const Model::ImageData& image_data, float alpha_cut_off, uint32_t num_mip_levels, int32_t size_alignment = -1)
{
    cyber_check(image_data.pData != nullptr);
    cyber_check(image_data.width > 0 && image_data.height > 0 && image_data.num_components > 0);

    const auto tex_format = get_image_data_texture_format(image_data);
    const auto& fmt_attribs = get_texture_format_attribs(tex_format);
    const auto src_stride = image_data.width * image_data.component_size * image_data.num_components;
    RenderObject::TextureData texture_data;
    texture_data.numSubResources = 1;
    texture_data.pSubResources = cyber_new_n<RenderObject::TextureSubResData>(texture_data.numSubResources);
    RenderObject::TextureSubResData& sub_res_data = texture_data.pSubResources[0];
    auto& stride = sub_res_data.stride;
    stride = align_up(uint64_t(image_data.width) * fmt_attribs.component_size * image_data.num_components, 4);
    uint8_t* data_buffer = cyber_new_n<uint8_t>(stride * image_data.height);
    sub_res_data.pData = data_buffer;

    TextureLoader::CopyPixelsAttribs copy_attribs;
    copy_attribs.width = image_data.width;
    copy_attribs.height = image_data.height;
    copy_attribs.component_size = image_data.component_size;
    copy_attribs.src_pixels = image_data.pData;
    copy_attribs.src_stride = src_stride;
    copy_attribs.src_comp_count = image_data.num_components;
    copy_attribs.dst_pixels = data_buffer;
    copy_attribs.dst_stride = stride;
    copy_attribs.dst_comp_count = fmt_attribs.num_components;
    TextureLoader::copy_pixels(copy_attribs);

    return texture_data;
}

static void free_gltf_texture_data(RenderObject::TextureData& texture_data)
{
    if (texture_data.pSubResources)
    {
        for (uint32_t i = 0; i < texture_data.numSubResources; ++i)
        {
            if (texture_data.pSubResources[i].pData)
            {
                cyber_free(const_cast<void*>(texture_data.pSubResources[i].pData));
            }
        }
        cyber_free(texture_data.pSubResources);
        texture_data.pSubResources = nullptr;
    }
}

uint32_t Model::add_texture(RenderObject::IRenderDevice* render_device, const ImageData& image)
{
    const auto new_texture_index = static_cast<uint32_t>(textures.size());

    TextureInfo texture_info;

    if(image.width > 0 && image.height > 0)
    {
        const auto tex_format = get_image_data_texture_format(image);

        RenderObject::TextureData texture_data = prepare_gltf_texture_data(image, 0.0f, 1);

        RenderObject::TextureCreateDesc texture_desc;
        texture_desc.m_name = image.name;
        texture_desc.m_dimension = TEXTURE_DIMENSION::TEX_DIMENSION_2D;
        texture_desc.m_width = image.width;
        texture_desc.m_height = image.height;
        texture_desc.m_usage = GRAPHICS_RESOURCE_USAGE_DEFAULT;
        texture_desc.m_bindFlags = GRAPHICS_RESOURCE_BIND_SHADER_RESOURCE;
        texture_desc.m_format = tex_format;
        texture_desc.m_mipLevels = 0;
        render_device->create_texture(texture_desc, &texture_data, &texture_info.texture);

        free_gltf_texture_data(texture_data);
    }
    textures.push_back(texture_info);
    return new_texture_index;
}

CYBER_END_NAMESPACE
CYBER_END_NAMESPACE
