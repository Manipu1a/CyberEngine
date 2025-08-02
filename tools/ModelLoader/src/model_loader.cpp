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
        return ""; // No extension found
    }
    return file_path.substr(last_dot + 1);
}

VALUE_TYPE TinyGltfComponentTypeToValueType(int component_type)
{
    switch (component_type)
    {
        case TINYGLTF_COMPONENT_TYPE_BYTE: return VALUE_TYPE_INT8;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: return VALUE_TYPE_UINT8;
        case TINYGLTF_COMPONENT_TYPE_SHORT: return VALUE_TYPE_INT16;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: return VALUE_TYPE_UINT16;
        case TINYGLTF_COMPONENT_TYPE_INT: return VALUE_TYPE_INT32;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: return VALUE_TYPE_UINT32;
        case TINYGLTF_COMPONENT_TYPE_FLOAT: return VALUE_TYPE_FLOAT32;
        case TINYGLTF_COMPONENT_TYPE_DOUBLE: return VALUE_TYPE_FLOAT64;
        default: return VALUE_TYPE_UNDEFINED;
    }
}

Model::Model(const ModelCreateInfo& create_info)
{
    num_texture_attributes = DefaultTextureAttributes.size();
    TextureAttributeDesc* tex_attribs = cyber_new_n<TextureAttributeDesc>(num_texture_attributes);
    for(uint32_t i = 0; i < DefaultTextureAttributes.size(); ++i)
    {
        tex_attribs[i] = DefaultTextureAttributes[i];
    }
    texture_attribute_descs = tex_attribs;
}

Model::Model(RenderObject::IRenderDevice* render_device, RenderObject::IDeviceContext* context, const ModelCreateInfo& create_info)
    : Model(create_info)
{
    load_from_file(render_device, context, create_info);
}

void Model::load_from_file(RenderObject::IRenderDevice* render_device, RenderObject::IDeviceContext* context, const ModelCreateInfo& create_info)
{
    if(create_info.file_path == nullptr)
    {
        cyber_assert(false, "Model file path is null.");
        return;
    }

    tinygltf::TinyGLTF gltf_context;
    //tinygltf::Model model;


    std::string input_file_path = create_info.file_path;
    std::string file_extension = get_file_extension(input_file_path);
    std::string base_dir = "";
    if(input_file_path.find_last_of("/\\") != std::string::npos)
    {
        base_dir = input_file_path.substr(0, input_file_path.find_last_of("/\\"));
    }
    base_dir += "/";

    std::string error;
    std::string warning;

    bool result = false;
    if(file_extension.compare("glb") == 0)
    {
        // assume the file is a binary GLTF (.glb)
        result = gltf_context.LoadBinaryFromFile(&model, &error, &warning, input_file_path.c_str());
    }
    else
    {
        // assume the file is a ascii GLTF (.gltf)
        result = gltf_context.LoadASCIIFromFile(&model, &error, &warning, input_file_path.c_str());
    }

    if(!result)
    {
        if(!warning.empty())
        {
            cyber_warn(false, "GLTF Warning: {0}", warning.c_str());
        }

        if(!error.empty())
        {
            CB_ERROR("GLTF Error: {0}", error.c_str());
        }
        else
        {
            CB_ERROR("Failed to load GLTF model from file: {0}", input_file_path.c_str());
        }
    }

    for(auto& scene : model.scenes)
    {
        for(size_t i = 0; i < scene.nodes.size(); ++i)
        {
            const tinygltf::Node& node = model.nodes[scene.nodes[i]];
            for(const auto& child_id : node.children)
            {
                const tinygltf::Node& child_node = model.nodes[child_id];
                // Process child nodes if needed
                load_node(model, child_id);
            }
        }
    }

    // load materials
    load_materials(model);
    load_textures(render_device, model, base_dir);
    /*
    for(size_t i = 0; i < model.meshes.size(); ++i)
    {
        const tinygltf::Mesh& mesh = model.meshes[i];
        auto& new_mesh = meshes.emplace_back();
        for(size_t primitive_idx = 0; primitive_idx < mesh.primitives.size(); ++primitive_idx)
        {
            const tinygltf::Primitive& primitive = mesh.primitives[primitive_idx];

            if(model.textures.size() > 0 && primitive.material >= 0)
            {
                const tinygltf::Material& material = model.materials[primitive.material];
                const tinygltf::Texture& texture = model.textures[material.pbrMetallicRoughness.baseColorTexture.index];
                if(texture.source > -1)
                {
                    const tinygltf::Image& image = model.images[texture.source];
                    auto image_uri = image.uri;
                    new_mesh.image_paths.push_back(image_uri);
                }
            }
        }
    }
    */
}

void Model::load_node(const tinygltf::Model& gltf_model, uint32_t node_index)
{
    if(node_index < gltf_model.nodes.size())
    {
        const tinygltf::Node& node = gltf_model.nodes[node_index];
        // Process the node as needed
        for(const auto& child_id : node.children)
        {
            load_node(gltf_model, child_id);
        }
        if (node.mesh >= 0 && node.mesh < gltf_model.meshes.size())
        {
            load_mesh(gltf_model, node.mesh);
        }
    }
}

void Model::load_mesh(const tinygltf::Model& gltf_model, uint32_t mesh_index)
{
    if(mesh_index < gltf_model.meshes.size())
    {
        const tinygltf::Mesh& mesh = gltf_model.meshes[mesh_index];
        auto& new_mesh = meshes.emplace_back();

        // Process the mesh as needed
        for(size_t primitive_idx = 0; primitive_idx < mesh.primitives.size(); ++primitive_idx)
        {
            const tinygltf::Primitive& primitive = mesh.primitives[primitive_idx];

            uint32_t index_start = static_cast<uint32_t>(indices_data.size());
            uint32_t vertex_start = static_cast<uint32_t>(model_data.size());
            uint32_t index_count = 0;
            uint32_t vertex_count = 0;

            // Accessing the POSITION attribute
            auto position_attribs = primitive.attributes.find("POSITION");
            if (position_attribs != primitive.attributes.end())
            {
                const tinygltf::Accessor& accessor = gltf_model.accessors[position_attribs->second];
                vertex_count = accessor.count;
                // Process vertex data
                model_data.resize(model_data.size() + vertex_count);

                const tinygltf::BufferView& buffer_view = model.bufferViews[accessor.bufferView];
                if (buffer_view.target == 0)
                {
                    continue; // Not a valid target for mesh data
                }
                
                const tinygltf::Buffer& buffer = model.buffers[buffer_view.buffer];
                auto stride = accessor.ByteStride(buffer_view);
                auto value_type = TinyGltfComponentTypeToValueType(accessor.componentType);
                const void* data = &buffer.data[accessor.byteOffset + buffer_view.byteOffset];
                const float* vertex_data = static_cast<const float*>(data);
                auto component_count = stride / sizeof(float);
                for (size_t j = 0; j < vertex_count; ++j)
                {
                    size_t offset = j * component_count;
                    VertexBasicAttribs& vertex = model_data[j + vertex_start];
                    vertex.pos = { vertex_data[offset], vertex_data[offset + 1], vertex_data[offset + 2] };
                }
            }

            auto normal_attribs = primitive.attributes.find("NORMAL");
            if (normal_attribs != primitive.attributes.end())
            {
                const tinygltf::Accessor& accessor = model.accessors[normal_attribs->second];
                auto vertex_count = accessor.count;

                if (accessor.bufferView < 0 || accessor.bufferView >= model.bufferViews.size())
                {
                    continue; // Invalid buffer view index
                }

                const tinygltf::BufferView& buffer_view = model.bufferViews[accessor.bufferView];
                const tinygltf::Buffer& buffer = model.buffers[buffer_view.buffer];
                auto stride = accessor.ByteStride(buffer_view);
                auto value_type = TinyGltfComponentTypeToValueType(accessor.componentType);
                const void* data = &buffer.data[accessor.byteOffset + buffer_view.byteOffset];
                const float* normal_data = static_cast<const float*>(data);
                auto component_count = stride / sizeof(float);
                for (size_t j = 0; j < vertex_count; ++j)
                {
                    size_t offset = j * component_count;
                    VertexBasicAttribs& vertex = model_data[j + vertex_start];
                    vertex.normal = { normal_data[offset], normal_data[offset + 1], normal_data[offset + 2] };
                }
            }

            auto tangent_attribs = primitive.attributes.find("TANGENT");
            if (tangent_attribs != primitive.attributes.end())
            {
                const tinygltf::Accessor& accessor = model.accessors[tangent_attribs->second];
                auto vertex_count = accessor.count;

                if (accessor.bufferView < 0 || accessor.bufferView >= model.bufferViews.size())
                {
                    continue; // Invalid buffer view index
                }

                const tinygltf::BufferView& buffer_view = model.bufferViews[accessor.bufferView];
                const tinygltf::Buffer& buffer = model.buffers[buffer_view.buffer];
                auto stride = accessor.ByteStride(buffer_view);
                auto value_type = TinyGltfComponentTypeToValueType(accessor.componentType);
                const void* data = &buffer.data[accessor.byteOffset + buffer_view.byteOffset];
                const float* tangent_data = static_cast<const float*>(data);
                auto component_count = stride / sizeof(float);
                for (size_t j = 0; j < vertex_count; ++j)
                {
                    size_t offset = j * component_count;
                    VertexBasicAttribs& vertex = model_data[j + vertex_start];
                    vertex.tangent = { tangent_data[offset], tangent_data[offset + 1], tangent_data[offset + 2] };
                }
            }
            
            auto uv_attribs = primitive.attributes.find("TEXCOORD_0");
            if( uv_attribs != primitive.attributes.end())
            {
                const tinygltf::Accessor& accessor = model.accessors[uv_attribs->second];
                auto vertex_count = accessor.count;

                if (accessor.bufferView < 0 || accessor.bufferView >= model.bufferViews.size())
                {
                    continue; // Invalid buffer view index
                }

                const tinygltf::BufferView& buffer_view = model.bufferViews[accessor.bufferView];
                const tinygltf::Buffer& buffer = model.buffers[buffer_view.buffer];
                auto stride = accessor.ByteStride(buffer_view);
                auto value_type = TinyGltfComponentTypeToValueType(accessor.componentType);
                const void* data = &buffer.data[accessor.byteOffset + buffer_view.byteOffset];
                const float* uv_data = static_cast<const float*>(data);
                auto component_count = stride / sizeof(float);
                for (size_t j = 0; j < vertex_count; ++j)
                {
                    size_t offset = j * component_count;
                    VertexBasicAttribs& vertex = model_data[j + vertex_start];
                    vertex.uv0 = { uv_data[offset], uv_data[offset + 1] };
                }
            }

            // Process Index
            if(primitive.indices >= 0)
            {
                const tinygltf::Accessor& accessor = model.accessors[primitive.indices];
                index_count = accessor.count;
                indices_data.resize(indices_data.size() + index_count);

                if (accessor.bufferView < 0 || accessor.bufferView >= model.bufferViews.size())
                {
                    continue; // Invalid buffer view index
                }

                const tinygltf::BufferView& buffer_view = model.bufferViews[accessor.bufferView];
                const tinygltf::Buffer& buffer = model.buffers[buffer_view.buffer];
                auto stride = accessor.ByteStride(buffer_view);
                auto value_type = TinyGltfComponentTypeToValueType(accessor.componentType);
                const void* data = &buffer.data[accessor.byteOffset + buffer_view.byteOffset];
                const uint16_t* index_data = static_cast<const uint16_t*>(data);
                for (size_t j = 0; j < index_count; ++j)
                {
                    indices_data[index_start + j] = static_cast<uint32_t>(index_data[j]) + vertex_start; // Adjust index to match vertex start
                }
            }

            new_mesh.primitives.emplace_back(Primitive{  index_start, 
                                                        index_count, 
                                                        vertex_count, 
                                                        primitive.material >= 0 ? (uint32_t)primitive.material : 0 });
            // Process other attributes like NORMAL, TEXCOORD_0, etc.
        }
    }
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
        const auto& gltf_image = gltf_model.images[gltf_texture.source];

        ImageData image_data;
        image_data.width = gltf_image.width;
        image_data.height = gltf_image.height;
        image_data.num_components = gltf_image.component;
        image_data.component_size = gltf_image.bits / 8;
        image_data.file_format = (gltf_image.width < 0 && gltf_image.height < 0) ? static_cast<TextureLoader::IMAGE_FILE_FORMAT>(gltf_image.pixel_type) : TextureLoader::IMAGE_FILE_FORMAT::IMAGE_FILE_FORMAT_UNKNOWN;
        image_data.pData = gltf_image.image.data();
        image_data.data_size = gltf_image.image.size();

        add_texture(render_device, image_data, gltf_texture.sampler);
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

uint32_t Model::add_texture(RenderObject::IRenderDevice* render_device, const ImageData& image, int gltf_sampler_id)
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

        texture_info.texture = render_device->create_texture(texture_desc, &texture_data);
    }
    textures.push_back(texture_info);
    return new_texture_index;
}

CYBER_END_NAMESPACE
CYBER_END_NAMESPACE
