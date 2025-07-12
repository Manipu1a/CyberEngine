#include "model_loader.h"
#include "log/Log.h"

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
void ModelLoader::load_from_file(RenderObject::IRenderDevice* render_device, RenderObject::IDeviceContext* context, const ModelCreateInfo& create_info)
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

    for(size_t i = 0; i < model.meshes.size(); ++i)
    {
        const tinygltf::Mesh& mesh = model.meshes[i];
        auto& new_mesh = meshes.emplace_back();
        for(size_t primitive_idx = 0; primitive_idx < mesh.primitives.size(); ++primitive_idx)
        {
            const tinygltf::Primitive& primitive = mesh.primitives[primitive_idx];
            // Accessing the POSITION attribute
            auto position_attribs = primitive.attributes.find("POSITION"); 
            if (position_attribs != primitive.attributes.end())
            {
                const tinygltf::Accessor& accessor = model.accessors[position_attribs->second];
                auto vertex_count = accessor.count;
                new_mesh.model_data.resize(vertex_count);

                if (accessor.bufferView < 0 || accessor.bufferView >= model.bufferViews.size())
                {
                    continue; // Invalid buffer view index
                }

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
                    Mesh::VertexBasicAttribs& vertex = new_mesh.model_data[j];
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
                    new_mesh.model_data[j].normal = { normal_data[offset], normal_data[offset + 1], normal_data[offset + 2] };
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
                    new_mesh.model_data[j].tangent = { tangent_data[offset], tangent_data[offset + 1], tangent_data[offset + 2] };
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
                    new_mesh.model_data[j].uv0 = { uv_data[offset], uv_data[offset + 1] };
                }
            }

            // Indices
            if(primitive.indices >= 0)
            {
                const tinygltf::Accessor& accessor = model.accessors[primitive.indices];
                auto index_count = accessor.count;
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
                    new_mesh.indices_data.push_back(index_data[j]);
                }
            }
            
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
}

CYBER_END_NAMESPACE
CYBER_END_NAMESPACE
