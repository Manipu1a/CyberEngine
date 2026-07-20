#include "asset/mesh_importer.h"

#include "asset/asset_hash.h"
#include "ofbx.h"

#define TINYGLTF_NOEXCEPTION
#define JSON_NOEXCEPTION
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_IMPLEMENTATION
#include "GLFW/tiny_gltf.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <cstring>
#include <fstream>
#include <limits>
#include <unordered_map>

namespace Cyber
{
    namespace
    {
        struct LegacyMeshAssetPayloadHeader
        {
            uint32_t magic = kMeshAssetPayloadMagic;
            uint32_t version = 1;
            uint32_t sourceExtensionSize = 0;
            uint32_t reserved = 0;
            uint64_t sourceDataOffset = 0;
            uint64_t sourceDataSize = 0;
        };

        struct Float3
        {
            float x = 0.0f;
            float y = 0.0f;
            float z = 0.0f;
        };

        struct Matrix4
        {
            float m[16] {};
        };

        std::string lowercase(std::string_view text)
        {
            std::string out(text);
            std::transform(out.begin(), out.end(), out.begin(),
                [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            return out;
        }

        void set_error(std::string* outError, std::string message)
        {
            if (outError)
                *outError = std::move(message);
        }

        Matrix4 identity_matrix()
        {
            Matrix4 result {};
            result.m[0] = result.m[5] = result.m[10] = result.m[15] = 1.0f;
            return result;
        }

        Matrix4 multiply(const Matrix4& lhs, const Matrix4& rhs)
        {
            Matrix4 result {};
            for (int column = 0; column < 4; ++column)
            {
                for (int row = 0; row < 4; ++row)
                {
                    for (int k = 0; k < 4; ++k)
                        result.m[row + column * 4] += lhs.m[row + k * 4] * rhs.m[k + column * 4];
                }
            }
            return result;
        }

        Float3 transform_point(const Matrix4& matrix, Float3 point)
        {
            return {
                matrix.m[0] * point.x + matrix.m[4] * point.y + matrix.m[8] * point.z + matrix.m[12],
                matrix.m[1] * point.x + matrix.m[5] * point.y + matrix.m[9] * point.z + matrix.m[13],
                matrix.m[2] * point.x + matrix.m[6] * point.y + matrix.m[10] * point.z + matrix.m[14]
            };
        }

        Float3 normalize(Float3 value)
        {
            const float length = std::sqrt(value.x * value.x + value.y * value.y + value.z * value.z);
            if (length <= std::numeric_limits<float>::epsilon())
                return {};
            return { value.x / length, value.y / length, value.z / length };
        }

        Float3 transform_direction(const Matrix4& matrix, Float3 direction)
        {
            return normalize({
                matrix.m[0] * direction.x + matrix.m[4] * direction.y + matrix.m[8] * direction.z,
                matrix.m[1] * direction.x + matrix.m[5] * direction.y + matrix.m[9] * direction.z,
                matrix.m[2] * direction.x + matrix.m[6] * direction.y + matrix.m[10] * direction.z
            });
        }

        Float3 subtract(Float3 lhs, Float3 rhs)
        {
            return { lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z };
        }

        Float3 add(Float3 lhs, Float3 rhs)
        {
            return { lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z };
        }

        Float3 cross(Float3 lhs, Float3 rhs)
        {
            return {
                lhs.y * rhs.z - lhs.z * rhs.y,
                lhs.z * rhs.x - lhs.x * rhs.z,
                lhs.x * rhs.y - lhs.y * rhs.x
            };
        }

        Float3 vertex_position(const CookedMeshVertex& vertex)
        {
            return { vertex.position[0], vertex.position[1], vertex.position[2] };
        }

        void set_vertex_position(CookedMeshVertex& vertex, Float3 value)
        {
            vertex.position[0] = value.x;
            vertex.position[1] = value.y;
            vertex.position[2] = value.z;
        }

        void set_vertex_normal(CookedMeshVertex& vertex, Float3 value)
        {
            vertex.normal[0] = value.x;
            vertex.normal[1] = value.y;
            vertex.normal[2] = value.z;
        }

        void expand_bounds(CookedMeshPrimitive& primitive, Float3 point)
        {
            for (int i = 0; i < 3; ++i)
            {
                const float value = (&point.x)[i];
                primitive.boundsMin[i] = std::min(primitive.boundsMin[i], value);
                primitive.boundsMax[i] = std::max(primitive.boundsMax[i], value);
            }
        }

        void initialize_bounds(CookedMeshPrimitive& primitive)
        {
            for (int i = 0; i < 3; ++i)
            {
                primitive.boundsMin[i] = std::numeric_limits<float>::max();
                primitive.boundsMax[i] = -std::numeric_limits<float>::max();
            }
        }

        void generate_missing_normals(CookedMeshData& data, uint32_t vertexStart,
                                      uint32_t vertexCount, uint32_t indexStart,
                                      uint32_t indexCount)
        {
            for (uint32_t i = indexStart; i + 2 < indexStart + indexCount; i += 3)
            {
                CookedMeshVertex& a = data.vertices[data.indices[i + 0]];
                CookedMeshVertex& b = data.vertices[data.indices[i + 1]];
                CookedMeshVertex& c = data.vertices[data.indices[i + 2]];
                const Float3 face = normalize(cross(
                    subtract(vertex_position(b), vertex_position(a)),
                    subtract(vertex_position(c), vertex_position(a))));
                set_vertex_normal(a, add({ a.normal[0], a.normal[1], a.normal[2] }, face));
                set_vertex_normal(b, add({ b.normal[0], b.normal[1], b.normal[2] }, face));
                set_vertex_normal(c, add({ c.normal[0], c.normal[1], c.normal[2] }, face));
            }

            for (uint32_t i = 0; i < vertexCount; ++i)
            {
                CookedMeshVertex& vertex = data.vertices[vertexStart + i];
                set_vertex_normal(vertex, normalize({ vertex.normal[0], vertex.normal[1], vertex.normal[2] }));
            }
        }

        Matrix4 from_fbx_matrix(const ofbx::DMatrix& source)
        {
            Matrix4 result {};
            for (int i = 0; i < 16; ++i)
                result.m[i] = static_cast<float>(source.m[i]);
            return result;
        }

        bool cook_fbx(const std::vector<uint8_t>& sourceBytes, CookedMeshData& outData,
                      std::string& outError)
        {
            ofbx::LoadFlags flags = ofbx::LoadFlags::IGNORE_BLEND_SHAPES |
                                    ofbx::LoadFlags::IGNORE_CAMERAS |
                                    ofbx::LoadFlags::IGNORE_LIGHTS |
                                    ofbx::LoadFlags::IGNORE_TEXTURES |
                                    ofbx::LoadFlags::IGNORE_SKIN |
                                    ofbx::LoadFlags::IGNORE_BONES |
                                    ofbx::LoadFlags::IGNORE_PIVOTS |
                                    ofbx::LoadFlags::IGNORE_ANIMATIONS |
                                    ofbx::LoadFlags::IGNORE_POSES |
                                    ofbx::LoadFlags::IGNORE_VIDEOS |
                                    ofbx::LoadFlags::IGNORE_LIMBS;

            std::unique_ptr<ofbx::IScene, void(*)(ofbx::IScene*)> scene(
                ofbx::load(sourceBytes.data(), sourceBytes.size(), static_cast<ofbx::u16>(flags)),
                [](ofbx::IScene* value) { if (value) value->destroy(); });
            if (!scene)
            {
                outError = std::string("OpenFBX failed: ") + ofbx::getError();
                return false;
            }

            outData.materials.emplace_back();
            std::unordered_map<const ofbx::Material*, uint32_t> materialIds;
            auto material_id = [&](const ofbx::Mesh& mesh, int partitionIndex) -> uint32_t
            {
                const ofbx::Material* sourceMaterial = partitionIndex < mesh.getMaterialCount()
                    ? mesh.getMaterial(partitionIndex) : nullptr;
                if (!sourceMaterial)
                    return 0;
                auto existing = materialIds.find(sourceMaterial);
                if (existing != materialIds.end())
                    return existing->second;

                CookedMeshMaterial material;
                const ofbx::Color diffuse = sourceMaterial->getDiffuseColor();
                const float factor = static_cast<float>(sourceMaterial->getDiffuseFactor());
                material.baseColorFactor[0] = diffuse.r * factor;
                material.baseColorFactor[1] = diffuse.g * factor;
                material.baseColorFactor[2] = diffuse.b * factor;
                const uint32_t id = static_cast<uint32_t>(outData.materials.size());
                outData.materials.push_back(material);
                materialIds.emplace(sourceMaterial, id);
                return id;
            };

            for (int meshIndex = 0; meshIndex < scene->getMeshCount(); ++meshIndex)
            {
                const ofbx::Mesh* sourceMesh = scene->getMesh(meshIndex);
                if (!sourceMesh)
                    continue;

                const ofbx::GeometryData& geometry = sourceMesh->getGeometryData();
                const ofbx::Vec3Attributes positions = geometry.getPositions();
                const ofbx::Vec3Attributes normals = geometry.getNormals();
                const ofbx::Vec2Attributes uvs = geometry.getUVs();
                if (!positions.values || positions.count <= 0)
                    continue;

                const Matrix4 transform = multiply(
                    from_fbx_matrix(sourceMesh->getGlobalTransform()),
                    from_fbx_matrix(sourceMesh->getGeometricMatrix()));
                const uint32_t vertexStart = static_cast<uint32_t>(outData.vertices.size());
                const uint32_t vertexCount = static_cast<uint32_t>(positions.count);
                const bool hasNormals = normals.values && normals.count >= positions.count;
                const bool hasUvs = uvs.values && uvs.count >= positions.count;
                outData.vertices.resize(outData.vertices.size() + vertexCount);

                for (int i = 0; i < positions.count; ++i)
                {
                    CookedMeshVertex& vertex = outData.vertices[vertexStart + i];
                    const ofbx::Vec3 position = positions.get(i);
                    set_vertex_position(vertex, transform_point(transform,
                        { position.x, position.y, position.z }));
                    if (hasNormals)
                    {
                        const ofbx::Vec3 normal = normals.get(i);
                        set_vertex_normal(vertex, transform_direction(transform,
                            { normal.x, normal.y, normal.z }));
                    }
                    if (hasUvs)
                    {
                        const ofbx::Vec2 uv = uvs.get(i);
                        vertex.uv0[0] = uv.x;
                        vertex.uv0[1] = uv.y;
                    }
                }

                CookedMeshRecord mesh;
                mesh.firstPrimitive = static_cast<uint32_t>(outData.primitives.size());
                std::strncpy(mesh.name, sourceMesh->name, sizeof(mesh.name) - 1);
                const uint32_t meshIndexStart = static_cast<uint32_t>(outData.indices.size());

                for (int partitionIndex = 0; partitionIndex < geometry.getPartitionCount(); ++partitionIndex)
                {
                    const ofbx::GeometryPartition partition = geometry.getPartition(partitionIndex);
                    CookedMeshPrimitive primitive;
                    primitive.firstIndex = static_cast<uint32_t>(outData.indices.size());
                    primitive.vertexCount = vertexCount;
                    primitive.materialIndex = material_id(*sourceMesh, partitionIndex);
                    initialize_bounds(primitive);
                    std::vector<int> triangleIndices(
                        static_cast<size_t>(std::max(1, partition.max_polygon_triangles)) * 3);

                    for (int polygonIndex = 0; polygonIndex < partition.polygon_count; ++polygonIndex)
                    {
                        const auto& polygon = partition.polygons[polygonIndex];
                        const ofbx::u32 count = ofbx::triangulate(
                            geometry, polygon, triangleIndices.data());
                        for (ofbx::u32 i = 0; i < count; ++i)
                        {
                            const uint32_t localIndex = static_cast<uint32_t>(triangleIndices[i]);
                            if (localIndex >= vertexCount)
                                continue;
                            outData.indices.push_back(vertexStart + localIndex);
                            expand_bounds(primitive, vertex_position(outData.vertices[vertexStart + localIndex]));
                        }
                    }

                    primitive.indexCount = static_cast<uint32_t>(outData.indices.size()) - primitive.firstIndex;
                    if (primitive.indexCount > 0)
                        outData.primitives.push_back(primitive);
                }

                mesh.primitiveCount = static_cast<uint32_t>(outData.primitives.size()) - mesh.firstPrimitive;
                if (mesh.primitiveCount == 0)
                {
                    outData.vertices.resize(vertexStart);
                    outData.indices.resize(meshIndexStart);
                    continue;
                }
                if (!hasNormals)
                    generate_missing_normals(outData, vertexStart, vertexCount,
                        meshIndexStart, static_cast<uint32_t>(outData.indices.size()) - meshIndexStart);
                outData.meshes.push_back(mesh);
            }

            if (outData.vertices.empty() || outData.indices.empty() || outData.meshes.empty())
            {
                outError = "FBX contains no renderable mesh data.";
                return false;
            }
            return true;
        }

        Matrix4 gltf_local_transform(const tinygltf::Node& node)
        {
            if (node.matrix.size() == 16)
            {
                Matrix4 result {};
                for (int i = 0; i < 16; ++i)
                    result.m[i] = static_cast<float>(node.matrix[i]);
                return result;
            }

            Matrix4 scale = identity_matrix();
            if (node.scale.size() == 3)
            {
                scale.m[0] = static_cast<float>(node.scale[0]);
                scale.m[5] = static_cast<float>(node.scale[1]);
                scale.m[10] = static_cast<float>(node.scale[2]);
            }

            Matrix4 rotation = identity_matrix();
            if (node.rotation.size() == 4)
            {
                const float x = static_cast<float>(node.rotation[0]);
                const float y = static_cast<float>(node.rotation[1]);
                const float z = static_cast<float>(node.rotation[2]);
                const float w = static_cast<float>(node.rotation[3]);
                rotation.m[0] = 1.0f - 2.0f * (y * y + z * z);
                rotation.m[1] = 2.0f * (x * y + z * w);
                rotation.m[2] = 2.0f * (x * z - y * w);
                rotation.m[4] = 2.0f * (x * y - z * w);
                rotation.m[5] = 1.0f - 2.0f * (x * x + z * z);
                rotation.m[6] = 2.0f * (y * z + x * w);
                rotation.m[8] = 2.0f * (x * z + y * w);
                rotation.m[9] = 2.0f * (y * z - x * w);
                rotation.m[10] = 1.0f - 2.0f * (x * x + y * y);
            }

            Matrix4 translation = identity_matrix();
            if (node.translation.size() == 3)
            {
                translation.m[12] = static_cast<float>(node.translation[0]);
                translation.m[13] = static_cast<float>(node.translation[1]);
                translation.m[14] = static_cast<float>(node.translation[2]);
            }
            return multiply(translation, multiply(rotation, scale));
        }

        struct GltfAccessorData
        {
            const uint8_t* data = nullptr;
            size_t count = 0;
            size_t stride = 0;
            int componentType = 0;
        };

        GltfAccessorData gltf_accessor(const tinygltf::Model& model, int accessorIndex)
        {
            GltfAccessorData result;
            if (accessorIndex < 0 || accessorIndex >= static_cast<int>(model.accessors.size()))
                return result;
            const auto& accessor = model.accessors[accessorIndex];
            if (accessor.bufferView < 0 || accessor.bufferView >= static_cast<int>(model.bufferViews.size()))
                return result;
            const auto& view = model.bufferViews[accessor.bufferView];
            if (view.buffer < 0 || view.buffer >= static_cast<int>(model.buffers.size()))
                return result;
            const auto& buffer = model.buffers[view.buffer];
            const size_t offset = accessor.byteOffset + view.byteOffset;
            if (offset >= buffer.data.size())
                return result;
            result.data = buffer.data.data() + offset;
            result.count = accessor.count;
            result.stride = static_cast<size_t>(accessor.ByteStride(view));
            result.componentType = accessor.componentType;
            return result;
        }

        void cook_gltf_materials(const tinygltf::Model& model, CookedMeshData& outData)
        {
            for (const auto& source : model.materials)
            {
                CookedMeshMaterial material;
                auto color = source.values.find("baseColorFactor");
                if (color != source.values.end() && color->second.number_array.size() >= 4)
                {
                    for (int i = 0; i < 4; ++i)
                        material.baseColorFactor[i] = static_cast<float>(color->second.number_array[i]);
                }
                auto metallic = source.values.find("metallicFactor");
                if (metallic != source.values.end())
                    material.metallicFactor = static_cast<float>(metallic->second.Factor());
                auto roughness = source.values.find("roughnessFactor");
                if (roughness != source.values.end())
                    material.roughnessFactor = static_cast<float>(roughness->second.Factor());
                outData.materials.push_back(material);
            }
            if (outData.materials.empty())
                outData.materials.emplace_back();
        }

        bool append_gltf_mesh(const tinygltf::Model& model, const tinygltf::Mesh& sourceMesh,
                              const Matrix4& transform, CookedMeshData& outData)
        {
            CookedMeshRecord mesh;
            mesh.firstPrimitive = static_cast<uint32_t>(outData.primitives.size());
            std::strncpy(mesh.name, sourceMesh.name.c_str(), sizeof(mesh.name) - 1);

            for (const auto& sourcePrimitive : sourceMesh.primitives)
            {
                auto positionIt = sourcePrimitive.attributes.find("POSITION");
                if (positionIt == sourcePrimitive.attributes.end())
                    continue;
                const GltfAccessorData positions = gltf_accessor(model, positionIt->second);
                if (!positions.data || positions.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT || positions.stride < 12)
                    continue;

                const uint32_t vertexStart = static_cast<uint32_t>(outData.vertices.size());
                const uint32_t vertexCount = static_cast<uint32_t>(positions.count);
                outData.vertices.resize(outData.vertices.size() + vertexCount);
                for (size_t i = 0; i < positions.count; ++i)
                {
                    const float* value = reinterpret_cast<const float*>(positions.data + i * positions.stride);
                    set_vertex_position(outData.vertices[vertexStart + i],
                        transform_point(transform, { value[0], value[1], value[2] }));
                }

                bool hasNormals = false;
                auto normalIt = sourcePrimitive.attributes.find("NORMAL");
                if (normalIt != sourcePrimitive.attributes.end())
                {
                    const GltfAccessorData normals = gltf_accessor(model, normalIt->second);
                    hasNormals = normals.data && normals.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT &&
                                 normals.count >= positions.count && normals.stride >= 12;
                    if (hasNormals)
                    {
                        for (size_t i = 0; i < positions.count; ++i)
                        {
                            const float* value = reinterpret_cast<const float*>(normals.data + i * normals.stride);
                            set_vertex_normal(outData.vertices[vertexStart + i],
                                transform_direction(transform, { value[0], value[1], value[2] }));
                        }
                    }
                }

                auto uvIt = sourcePrimitive.attributes.find("TEXCOORD_0");
                if (uvIt != sourcePrimitive.attributes.end())
                {
                    const GltfAccessorData uvs = gltf_accessor(model, uvIt->second);
                    if (uvs.data && uvs.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT &&
                        uvs.count >= positions.count && uvs.stride >= 8)
                    {
                        for (size_t i = 0; i < positions.count; ++i)
                        {
                            const float* value = reinterpret_cast<const float*>(uvs.data + i * uvs.stride);
                            outData.vertices[vertexStart + i].uv0[0] = value[0];
                            outData.vertices[vertexStart + i].uv0[1] = value[1];
                        }
                    }
                }

                auto tangentIt = sourcePrimitive.attributes.find("TANGENT");
                if (tangentIt != sourcePrimitive.attributes.end())
                {
                    const GltfAccessorData tangents = gltf_accessor(model, tangentIt->second);
                    if (tangents.data && tangents.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT &&
                        tangents.count >= positions.count && tangents.stride >= 12)
                    {
                        for (size_t i = 0; i < positions.count; ++i)
                        {
                            const float* value = reinterpret_cast<const float*>(tangents.data + i * tangents.stride);
                            const Float3 tangent = transform_direction(transform, { value[0], value[1], value[2] });
                            outData.vertices[vertexStart + i].tangent[0] = tangent.x;
                            outData.vertices[vertexStart + i].tangent[1] = tangent.y;
                            outData.vertices[vertexStart + i].tangent[2] = tangent.z;
                        }
                    }
                }

                CookedMeshPrimitive primitive;
                primitive.firstIndex = static_cast<uint32_t>(outData.indices.size());
                primitive.vertexCount = vertexCount;
                primitive.materialIndex = sourcePrimitive.material >= 0
                    ? static_cast<uint32_t>(sourcePrimitive.material) : 0;
                initialize_bounds(primitive);

                if (sourcePrimitive.indices >= 0)
                {
                    const GltfAccessorData indices = gltf_accessor(model, sourcePrimitive.indices);
                    for (size_t i = 0; indices.data && i < indices.count; ++i)
                    {
                        uint32_t value = 0;
                        const uint8_t* source = indices.data + i * indices.stride;
                        if (indices.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
                            value = *source;
                        else if (indices.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
                            std::memcpy(&value, source, sizeof(uint16_t));
                        else if (indices.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
                            std::memcpy(&value, source, sizeof(uint32_t));
                        else
                            break;
                        if (value < vertexCount)
                            outData.indices.push_back(vertexStart + value);
                    }
                }
                else
                {
                    for (uint32_t i = 0; i < vertexCount; ++i)
                        outData.indices.push_back(vertexStart + i);
                }

                primitive.indexCount = static_cast<uint32_t>(outData.indices.size()) - primitive.firstIndex;
                if (primitive.indexCount == 0)
                {
                    outData.vertices.resize(vertexStart);
                    continue;
                }
                for (uint32_t i = primitive.firstIndex; i < primitive.firstIndex + primitive.indexCount; ++i)
                    expand_bounds(primitive, vertex_position(outData.vertices[outData.indices[i]]));
                if (!hasNormals)
                    generate_missing_normals(outData, vertexStart, vertexCount,
                        primitive.firstIndex, primitive.indexCount);
                outData.primitives.push_back(primitive);
            }

            mesh.primitiveCount = static_cast<uint32_t>(outData.primitives.size()) - mesh.firstPrimitive;
            if (mesh.primitiveCount > 0)
            {
                outData.meshes.push_back(mesh);
                return true;
            }
            return false;
        }

        void append_gltf_node(const tinygltf::Model& model, int nodeIndex,
                              const Matrix4& parentTransform, CookedMeshData& outData)
        {
            if (nodeIndex < 0 || nodeIndex >= static_cast<int>(model.nodes.size()))
                return;
            const auto& node = model.nodes[nodeIndex];
            const Matrix4 world = multiply(parentTransform, gltf_local_transform(node));
            if (node.mesh >= 0 && node.mesh < static_cast<int>(model.meshes.size()))
                append_gltf_mesh(model, model.meshes[node.mesh], world, outData);
            for (int child : node.children)
                append_gltf_node(model, child, world, outData);
        }

        bool cook_gltf(const std::filesystem::path& sourcePath, CookedMeshData& outData,
                       std::string& outError)
        {
            tinygltf::TinyGLTF loader;
            loader.SetImageLoader(
                [](tinygltf::Image*, int, std::string*, std::string*, int, int,
                   const unsigned char*, int, void*) { return true; },
                nullptr);
            tinygltf::Model model;
            std::string warning;
            bool loaded = false;
            if (lowercase(sourcePath.extension().string()) == ".glb")
                loaded = loader.LoadBinaryFromFile(&model, &outError, &warning, sourcePath.string());
            else
                loaded = loader.LoadASCIIFromFile(&model, &outError, &warning, sourcePath.string());
            if (!loaded)
            {
                if (outError.empty())
                    outError = warning.empty() ? "tinygltf failed to load source." : warning;
                return false;
            }

            cook_gltf_materials(model, outData);
            const Matrix4 identity = identity_matrix();
            if (model.defaultScene >= 0 && model.defaultScene < static_cast<int>(model.scenes.size()))
            {
                for (int node : model.scenes[model.defaultScene].nodes)
                    append_gltf_node(model, node, identity, outData);
            }
            else
            {
                for (const auto& scene : model.scenes)
                    for (int node : scene.nodes)
                        append_gltf_node(model, node, identity, outData);
            }

            if (outData.vertices.empty() || outData.indices.empty() || outData.meshes.empty())
            {
                outError = "glTF contains no renderable mesh data.";
                return false;
            }
            return true;
        }

        bool cook_source(const std::filesystem::path& sourcePath,
                         const std::vector<uint8_t>& sourceBytes,
                         CookedMeshData& outData, std::string& outError)
        {
            outData = {};
            const std::string extension = lowercase(sourcePath.extension().string());
            if (extension == ".fbx")
                return cook_fbx(sourceBytes, outData, outError);
            if (extension == ".gltf" || extension == ".glb")
                return cook_gltf(sourcePath, outData, outError);
            outError = "Unsupported mesh source extension.";
            return false;
        }

        template <typename T>
        uint64_t section_size(const std::vector<T>& values)
        {
            return static_cast<uint64_t>(values.size()) * sizeof(T);
        }

        bool write_cooked_asset(const AssetImportRequest& request,
                                const std::vector<uint8_t>& sourceBytes,
                                CookedMeshData& cookedData,
                                AssetGuid assetGuid,
                                AssetFileHeader& outHeader)
        {
            const std::string sourceExtension = lowercase(request.sourcePath.extension().string());
            if (sourceExtension.size() > 64)
                return false;

            MeshAssetPayloadHeader payload;
            payload.sourceExtensionSize = static_cast<uint32_t>(sourceExtension.size());
            uint64_t cursor = sizeof(payload) + sourceExtension.size();
            payload.verticesOffset = cursor;
            payload.vertexCount = cookedData.vertices.size();
            cursor += section_size(cookedData.vertices);
            payload.indicesOffset = cursor;
            payload.indexCount = cookedData.indices.size();
            cursor += section_size(cookedData.indices);
            payload.meshesOffset = cursor;
            payload.meshCount = cookedData.meshes.size();
            cursor += section_size(cookedData.meshes);
            payload.primitivesOffset = cursor;
            payload.primitiveCount = cookedData.primitives.size();
            cursor += section_size(cookedData.primitives);
            payload.materialsOffset = cursor;
            payload.materialCount = cookedData.materials.size();
            cursor += section_size(cookedData.materials);
            payload.texturesOffset = cursor;
            payload.textureCount = cookedData.textures.size();
            cursor += static_cast<uint64_t>(cookedData.textures.size()) * sizeof(CookedMeshTextureRecord);
            payload.textureDataOffset = cursor;

            std::vector<CookedMeshTextureRecord> textureRecords;
            textureRecords.reserve(cookedData.textures.size());
            for (auto& texture : cookedData.textures)
            {
                texture.record.dataOffset = cursor;
                texture.record.dataSize = texture.bytes.size();
                textureRecords.push_back(texture.record);
                cursor += texture.bytes.size();
            }
            payload.textureDataSize = cursor - payload.textureDataOffset;

            AssetFileHeader fileHeader;
            fileHeader.assetType = AssetType::Mesh;
            fileHeader.assetGuid = assetGuid.IsValid() ? assetGuid : AssetGuid::Create();
            fileHeader.contentHash = AssetHash::HashBytes(sourceBytes.data(), sourceBytes.size());
            fileHeader.dependencyHash = AssetHash::HashString(request.sourcePath.generic_string());
            fileHeader.cookerVersion = kMeshImporterVersion;
            fileHeader.platformTag = kAnyAssetPlatform;
            fileHeader.payloadOffset = sizeof(AssetFileHeader);
            fileHeader.payloadSize = cursor;

            std::error_code ec;
            if (!request.destinationPath.parent_path().empty())
                std::filesystem::create_directories(request.destinationPath.parent_path(), ec);
            if (ec)
                return false;

            std::ofstream file(request.destinationPath, std::ios::binary | std::ios::trunc);
            if (!file)
                return false;
            auto write_vector = [&](const auto& values)
            {
                if (!values.empty())
                    file.write(reinterpret_cast<const char*>(values.data()),
                               static_cast<std::streamsize>(values.size() * sizeof(values[0])));
            };

            file.write(reinterpret_cast<const char*>(&fileHeader), sizeof(fileHeader));
            file.write(reinterpret_cast<const char*>(&payload), sizeof(payload));
            file.write(sourceExtension.data(), static_cast<std::streamsize>(sourceExtension.size()));
            write_vector(cookedData.vertices);
            write_vector(cookedData.indices);
            write_vector(cookedData.meshes);
            write_vector(cookedData.primitives);
            write_vector(cookedData.materials);
            write_vector(textureRecords);
            for (const auto& texture : cookedData.textures)
                write_vector(texture.bytes);
            if (!file.good())
                return false;
            outHeader = fileHeader;
            return true;
        }

        bool section_inside(uint64_t offset, uint64_t count, uint64_t stride, uint64_t payloadSize)
        {
            if (stride != 0 && count > std::numeric_limits<uint64_t>::max() / stride)
                return false;
            const uint64_t size = count * stride;
            return offset <= payloadSize && size <= payloadSize - offset;
        }

        template <typename T>
        bool read_section(std::ifstream& file, const AssetFileHeader& header,
                          uint64_t offset, uint64_t count, std::vector<T>& out)
        {
            if (count > std::numeric_limits<size_t>::max())
                return false;
            out.resize(static_cast<size_t>(count));
            if (out.empty())
                return true;
            file.seekg(static_cast<std::streamoff>(header.payloadOffset + offset), std::ios::beg);
            file.read(reinterpret_cast<char*>(out.data()),
                      static_cast<std::streamsize>(out.size() * sizeof(T)));
            return static_cast<bool>(file);
        }

        bool read_legacy_header(const std::filesystem::path& path, AssetFileHeader& fileHeader,
                                LegacyMeshAssetPayloadHeader& payload, std::string& extension)
        {
            std::ifstream file(path, std::ios::binary);
            if (!file)
                return false;
            file.read(reinterpret_cast<char*>(&fileHeader), sizeof(fileHeader));
            if (!file || !fileHeader.IsCompatible(AssetType::Mesh, kAssetFileFormatVersion))
                return false;
            file.seekg(static_cast<std::streamoff>(fileHeader.payloadOffset), std::ios::beg);
            file.read(reinterpret_cast<char*>(&payload), sizeof(payload));
            if (!file || payload.magic != kMeshAssetPayloadMagic || payload.version != 1 ||
                payload.sourceExtensionSize > 64)
                return false;
            if (payload.sourceDataOffset < sizeof(payload) + payload.sourceExtensionSize ||
                !section_inside(payload.sourceDataOffset, payload.sourceDataSize, 1, fileHeader.payloadSize))
                return false;
            extension.resize(payload.sourceExtensionSize);
            if (!extension.empty())
                file.read(extension.data(), static_cast<std::streamsize>(extension.size()));
            return static_cast<bool>(file);
        }
    }

    bool MeshImporter::Import(const AssetImportRequest& request, AssetImportResult& outResult) const
    {
        outResult = {};
        if (request.sourcePath.empty() || request.destinationPath.empty())
        {
            outResult.error = "Mesh import requires source and destination paths.";
            return false;
        }
        if (!IsSupportedSourceExtension(request.sourcePath.extension().string()))
        {
            outResult.error = "Unsupported mesh source extension.";
            return false;
        }

        std::vector<uint8_t> sourceBytes;
        if (!ReadFileBytes(request.sourcePath, sourceBytes))
        {
            outResult.error = "Failed to read mesh source file.";
            return false;
        }

        CookedMeshData cookedData;
        if (!cook_source(request.sourcePath, sourceBytes, cookedData, outResult.error))
            return false;

        AssetFileHeader fileHeader;
        if (!write_cooked_asset(request, sourceBytes, cookedData, request.existingGuid, fileHeader))
        {
            outResult.error = "Failed to write cooked mesh asset.";
            return false;
        }

        AssetRegistryRecord record;
        record.guid = fileHeader.assetGuid;
        record.type = AssetType::Mesh;
        record.assetPath = AssetRegistry::MakeStoredPath(request.destinationPath, request.contentRoot);
        record.displayName = request.destinationPath.stem().string();
        record.sourcePath = AssetRegistry::MakeStoredPath(request.sourcePath, request.contentRoot);
        record.sourceHash = fileHeader.contentHash;
        record.editorAssetHash = AssetHash::Combine(fileHeader.contentHash, fileHeader.dependencyHash);
        record.importerVersion = Version();
        record.assetFormatVersion = kAssetFileFormatVersion;
        outResult.registryRecord = std::move(record);
        return true;
    }

    bool MeshImporter::IsSupportedSourceExtension(std::string_view extension)
    {
        const std::string ext = lowercase(extension);
        static constexpr std::array<std::string_view, 3> extensions {{ ".fbx", ".gltf", ".glb" }};
        return std::find(extensions.begin(), extensions.end(), ext) != extensions.end();
    }

    bool MeshImporter::ReadInfo(const std::filesystem::path& path, MeshEditorAssetInfo& outInfo)
    {
        outInfo = {};
        std::ifstream file(path, std::ios::binary);
        if (!file)
            return false;
        AssetFileHeader fileHeader;
        file.read(reinterpret_cast<char*>(&fileHeader), sizeof(fileHeader));
        if (!file || !fileHeader.IsCompatible(AssetType::Mesh, kAssetFileFormatVersion) ||
            fileHeader.payloadOffset < sizeof(AssetFileHeader))
            return false;

        file.seekg(static_cast<std::streamoff>(fileHeader.payloadOffset), std::ios::beg);
        uint32_t prefix[2] {};
        file.read(reinterpret_cast<char*>(prefix), sizeof(prefix));
        if (!file || prefix[0] != kMeshAssetPayloadMagic)
            return false;

        if (prefix[1] == 1)
        {
            LegacyMeshAssetPayloadHeader legacy;
            std::string extension;
            if (!read_legacy_header(path, fileHeader, legacy, extension))
                return false;
            outInfo.fileHeader = fileHeader;
            outInfo.payloadHeader.version = 1;
            outInfo.payloadHeader.sourceExtensionSize = legacy.sourceExtensionSize;
            outInfo.sourceExtension = std::move(extension);
            outInfo.isCooked = false;
            return true;
        }
        if (prefix[1] != kMeshAssetPayloadVersion || fileHeader.payloadSize < sizeof(MeshAssetPayloadHeader))
            return false;

        file.seekg(static_cast<std::streamoff>(fileHeader.payloadOffset), std::ios::beg);
        MeshAssetPayloadHeader payload;
        file.read(reinterpret_cast<char*>(&payload), sizeof(payload));
        if (!file || payload.magic != kMeshAssetPayloadMagic ||
            payload.version != kMeshAssetPayloadVersion || payload.sourceExtensionSize > 64 ||
            payload.vertexStride != sizeof(CookedMeshVertex))
            return false;
        std::string extension(payload.sourceExtensionSize, '\0');
        if (!extension.empty())
            file.read(extension.data(), static_cast<std::streamsize>(extension.size()));
        if (!file)
            return false;
        outInfo.fileHeader = fileHeader;
        outInfo.payloadHeader = payload;
        outInfo.sourceExtension = std::move(extension);
        outInfo.isCooked = true;
        return true;
    }

    bool MeshImporter::ReadCookedData(const std::filesystem::path& path,
                                      CookedMeshData& outData, std::string* outError)
    {
        outData = {};
        if (outError)
            outError->clear();
        MeshEditorAssetInfo info;
        if (!ReadInfo(path, info))
        {
            set_error(outError, "Invalid mesh asset.");
            return false;
        }
        if (!info.isCooked)
        {
            set_error(outError, "Legacy mesh asset payload v1 must be reimported.");
            return false;
        }

        const auto& p = info.payloadHeader;
        const uint64_t size = info.fileHeader.payloadSize;
        if (!section_inside(p.verticesOffset, p.vertexCount, sizeof(CookedMeshVertex), size) ||
            !section_inside(p.indicesOffset, p.indexCount, sizeof(uint32_t), size) ||
            !section_inside(p.meshesOffset, p.meshCount, sizeof(CookedMeshRecord), size) ||
            !section_inside(p.primitivesOffset, p.primitiveCount, sizeof(CookedMeshPrimitive), size) ||
            !section_inside(p.materialsOffset, p.materialCount, sizeof(CookedMeshMaterial), size) ||
            !section_inside(p.texturesOffset, p.textureCount, sizeof(CookedMeshTextureRecord), size) ||
            !section_inside(p.textureDataOffset, p.textureDataSize, 1, size))
        {
            set_error(outError, "Cooked mesh asset contains an invalid section range.");
            return false;
        }

        std::ifstream file(path, std::ios::binary);
        std::vector<CookedMeshTextureRecord> textureRecords;
        if (!file ||
            !read_section(file, info.fileHeader, p.verticesOffset, p.vertexCount, outData.vertices) ||
            !read_section(file, info.fileHeader, p.indicesOffset, p.indexCount, outData.indices) ||
            !read_section(file, info.fileHeader, p.meshesOffset, p.meshCount, outData.meshes) ||
            !read_section(file, info.fileHeader, p.primitivesOffset, p.primitiveCount, outData.primitives) ||
            !read_section(file, info.fileHeader, p.materialsOffset, p.materialCount, outData.materials) ||
            !read_section(file, info.fileHeader, p.texturesOffset, p.textureCount, textureRecords))
        {
            set_error(outError, "Failed to read cooked mesh sections.");
            return false;
        }

        for (const auto& record : textureRecords)
        {
            if (!section_inside(record.dataOffset, record.dataSize, 1, size) ||
                record.dataOffset < p.textureDataOffset ||
                record.dataOffset + record.dataSize > p.textureDataOffset + p.textureDataSize)
            {
                set_error(outError, "Cooked mesh texture range is invalid.");
                return false;
            }
            CookedMeshTexture texture;
            texture.record = record;
            texture.bytes.resize(static_cast<size_t>(record.dataSize));
            if (!texture.bytes.empty())
            {
                file.seekg(static_cast<std::streamoff>(info.fileHeader.payloadOffset + record.dataOffset), std::ios::beg);
                file.read(reinterpret_cast<char*>(texture.bytes.data()),
                          static_cast<std::streamsize>(texture.bytes.size()));
                if (!file)
                {
                    set_error(outError, "Failed to read cooked mesh texture data.");
                    return false;
                }
            }
            outData.textures.push_back(std::move(texture));
        }

        for (uint32_t index : outData.indices)
        {
            if (index >= outData.vertices.size())
            {
                set_error(outError, "Cooked mesh contains an invalid vertex index.");
                return false;
            }
        }
        for (const auto& mesh : outData.meshes)
        {
            if (mesh.firstPrimitive > outData.primitives.size() ||
                mesh.primitiveCount > outData.primitives.size() - mesh.firstPrimitive)
            {
                set_error(outError, "Cooked mesh contains an invalid primitive range.");
                return false;
            }
        }
        for (const auto& primitive : outData.primitives)
        {
            if (primitive.firstIndex > outData.indices.size() ||
                primitive.indexCount > outData.indices.size() - primitive.firstIndex ||
                primitive.materialIndex >= outData.materials.size())
            {
                set_error(outError, "Cooked mesh primitive references invalid data.");
                return false;
            }
        }
        return !outData.vertices.empty() && !outData.indices.empty() && !outData.meshes.empty();
    }

    bool ReadCookedMeshAsset(const std::filesystem::path& path,
                             CookedMeshData& outData, std::string* outError)
    {
        return MeshImporter::ReadCookedData(path, outData, outError);
    }

    bool MeshImporter::ReadEmbeddedSource(const std::filesystem::path& path,
                                          MeshEditorAssetInfo& outInfo,
                                          std::vector<uint8_t>& outBytes)
    {
        outBytes.clear();
        AssetFileHeader fileHeader;
        LegacyMeshAssetPayloadHeader legacy;
        std::string extension;
        if (!read_legacy_header(path, fileHeader, legacy, extension))
            return false;
        std::ifstream file(path, std::ios::binary);
        file.seekg(static_cast<std::streamoff>(fileHeader.payloadOffset + legacy.sourceDataOffset), std::ios::beg);
        outBytes.resize(static_cast<size_t>(legacy.sourceDataSize));
        if (!outBytes.empty())
            file.read(reinterpret_cast<char*>(outBytes.data()), static_cast<std::streamsize>(outBytes.size()));
        if (!file)
            return false;
        outInfo = {};
        outInfo.fileHeader = fileHeader;
        outInfo.payloadHeader.version = 1;
        outInfo.sourceExtension = std::move(extension);
        outInfo.isCooked = false;
        return true;
    }

    bool MeshImporter::WriteEmbeddedSourceToCache(const std::filesystem::path& path,
                                                  const std::filesystem::path& cacheRoot,
                                                  std::filesystem::path& outPath)
    {
        outPath.clear();
        MeshEditorAssetInfo info;
        std::vector<uint8_t> bytes;
        if (!ReadEmbeddedSource(path, info, bytes))
            return false;
        std::error_code ec;
        std::filesystem::create_directories(cacheRoot, ec);
        if (ec)
            return false;
        outPath = cacheRoot / (info.fileHeader.assetGuid.ToString() + lowercase(info.sourceExtension));
        std::ofstream file(outPath, std::ios::binary | std::ios::trunc);
        if (!file)
            return false;
        if (!bytes.empty())
            file.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
        return file.good();
    }

    bool MeshImporter::ReadFileBytes(const std::filesystem::path& path,
                                     std::vector<uint8_t>& outBytes)
    {
        outBytes.clear();
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (!file)
            return false;
        const std::streamoff size = file.tellg();
        if (size < 0)
            return false;
        outBytes.resize(static_cast<size_t>(size));
        file.seekg(0, std::ios::beg);
        if (!outBytes.empty())
            file.read(reinterpret_cast<char*>(outBytes.data()), static_cast<std::streamsize>(outBytes.size()));
        return file.good() || file.eof();
    }
}
