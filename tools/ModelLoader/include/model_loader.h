#pragma once
#include "core/Core.h"
#include "platform/configure.h"
#include "eastl/vector.h"
#include "math/advanced_math.hpp"
#include "graphics/interface/render_device.hpp"
#include "image.h"
#include "GLFW/tiny_gltf.h"


CYBER_BEGIN_NAMESPACE(Cyber)
CYBER_BEGIN_NAMESPACE(ModelLoader)

struct TextureAttributeDesc
{
    const char* name = nullptr;
    uint32_t index = 0;
};

static constexpr char BaseColorTextureName[] = "baseColorTexture";
static constexpr char MetallicRoughnessTextureName[] = "metallicRoughnessTexture";
static constexpr char NormalTextureName[] = "normalTexture";
static constexpr char OcclusionTextureName[] = "occlusionTexture";
static constexpr char EmissiveTextureName[] = "emissiveTexture";
static constexpr char DiffuseTextureName[] = "diffuseTexture";
static constexpr char SpecularGlossinessTextureName[] = "specularGlossinessTexture";

static constexpr uint32_t DefaultBaseColorTextureAttribId = 0;
static constexpr uint32_t DefaultMetallicRoughnessTextureAttribId = 1;
static constexpr uint32_t DefaultNormalTextureAttribId = 2;
static constexpr uint32_t DefaultOcclusionTextureAttribId = 3;
static constexpr uint32_t DefaultEmissiveTextureAttribId = 4;
static constexpr uint32_t DefaultDiffuseTextureAttribId = 0;
static constexpr uint32_t DefaultSpecularGlossinessTextureAttribId = 1;

static constexpr std::array<TextureAttributeDesc, 7> DefaultTextureAttributes = {
    TextureAttributeDesc{ BaseColorTextureName, DefaultBaseColorTextureAttribId },
    TextureAttributeDesc{ MetallicRoughnessTextureName, DefaultMetallicRoughnessTextureAttribId },
    TextureAttributeDesc{ NormalTextureName, DefaultNormalTextureAttribId },
    TextureAttributeDesc{ OcclusionTextureName, DefaultOcclusionTextureAttribId },
    TextureAttributeDesc{ EmissiveTextureName, DefaultEmissiveTextureAttribId },
    TextureAttributeDesc{ DiffuseTextureName, DefaultDiffuseTextureAttribId },
    TextureAttributeDesc{ SpecularGlossinessTextureName, DefaultSpecularGlossinessTextureAttribId },
};

struct Material
{
    Material() noexcept
    {
        texture_ids.fill(-1);
        for(size_t i = 0; i < _countof(attribs.uv_scale_bias); ++i)
        {
            attribs.uv_scale_bias[i] = float4(1.0f, 1.0f, 0.0f, 0.0f); // Default UV scale and bias
        }
    }
    enum PBR_WORKFLOW
    {
        PBR_WORKFLOW_METALL_ROUGH = 0,
        PBR_WORKFLOW_SPEC_GLOSS
    };

    enum ALPHA_MODE
    {
        ALPHA_MODE_OPAQUE = 0,
        ALPHA_MODE_MASK,
        ALPHA_MODE_BLEND,
        ALPHA_MODE_NUM_MODES
    };

    static constexpr uint32_t num_texture_attributes = 5;

    bool has_base_color_texture() const
    {
        return texture_ids[DefaultBaseColorTextureAttribId] != -1;
    }
    bool has_metallic_roughness_texture() const
    {
        return texture_ids[DefaultMetallicRoughnessTextureAttribId] != -1;
    }
    bool has_normal_map() const
    {
        return texture_ids[DefaultNormalTextureAttribId] != -1;
    }
    bool has_occlusion_texture() const
    {
        return texture_ids[DefaultOcclusionTextureAttribId] != -1;
    }
    bool has_emissive_texture() const
    {
        return texture_ids[DefaultEmissiveTextureAttribId] != -1;
    }
    bool has_diffuse_texture() const
    {
        return texture_ids[DefaultDiffuseTextureAttribId] != -1;
    }
    bool has_specular_glossiness_texture() const
    {
        return texture_ids[DefaultSpecularGlossinessTextureAttribId] != -1;
    }
    
    struct MaterialAttribs
    {
        float4 base_color_factor = { 1.0f, 1.0f, 1.0f, 1.0f }; // Base color factor (RGBA)
        float4 emissive_factor = { 0.0f, 0.0f, 0.0f, 1.0f }; // Emissive color factor (RGBA)
        float4 specular_factor = { 1.0f, 1.0f, 1.0f, 1.0f }; // Specular color factor (RGBA)

        int work_flow = PBR_WORKFLOW_METALL_ROUGH;
        float uv_selector0 = -1;
        float uv_selector1 = -1;
        float uv_selector2 = -1;
        float uv_selector3 = -1;
        float uv_selector4 = -1;

        float texture_slice0 = 0.0f;
        float texture_slice1 = 0.0f;
        float texture_slice2 = 0.0f;
        float texture_slice3 = 0.0f;
        float texture_slice4 = 0.0f;

        float metallic_factor = 1.0f; // Metallic factor (0.0 to 1.0)
        float roughness_factor = 1.0f; // Roughness factor (0.0 to 1.0)
        int32_t alpha_mode = ALPHA_MODE::ALPHA_MODE_OPAQUE;
        float alpha_cutoff = 0.5f; // Alpha cutoff value for ALPHA_MODE_MASK
        float dummy0 = 0.0f; // Padding for alignment

        float4 uv_scale_bias[num_texture_attributes];

        float4 custom_data = float4(0.0f, 0.0f, 0.0f, 0.0f); // Custom data for additional attributes
    };

    MaterialAttribs attribs;

    bool double_sided = false;
    std::array<int, num_texture_attributes> texture_ids = {};
 };

struct Primitive
{
    uint32_t first_index;
    uint32_t index_count;
    uint32_t vertex_count;
    uint32_t material_id;

    BoundBox bound_box;

    Primitive(uint32_t _first_index, uint32_t _index_count, uint32_t _vertex_count, uint32_t _material_id, const float3& bb_min, const float3& bb_max)
        : first_index(_first_index), index_count(_index_count), vertex_count(_vertex_count), material_id(_material_id), bound_box(bb_min, bb_max) {}
};

struct Mesh
{
    std::string name;
    std::vector<Primitive> primitives;
    BoundBox bound_box;
    std::vector<std::string> image_paths; // Paths to textures used by the mesh

    void update_bounding_box()
    {
        if(primitives.size() > 0)
        {
            bound_box = primitives[0].bound_box;
            for (const auto& primitive : primitives)
            {
                bound_box.Min = Math::min(bound_box.Min, primitive.bound_box.Min);
                bound_box.Max = Math::max(bound_box.Max, primitive.bound_box.Max);
            }
        }
    }
};

struct ModelCreateInfo
{
    const char* file_path = nullptr; // Path to the model file
};

class Model
{
public:
    Model(const ModelCreateInfo& create_info);

    Model(RenderObject::IRenderDevice* render_device, RenderObject::IDeviceContext* context, const ModelCreateInfo& create_info);
    //static tinygltf::Model create_model(const ModelCreateInfo& create_info);

    void load_from_file(RenderObject::IRenderDevice* render_device, RenderObject::IDeviceContext* context, const ModelCreateInfo& create_info);

    BoundBox compute_bounding_box(const float4x4& model_transform) const;

    const tinygltf::Model& get_model() const
    {
        return model;
    }

    bool is_valid() const
    {
        return !model.meshes.empty();
    }

    const std::vector<Mesh>& get_meshes() const
    {
        return meshes;
    }

    const std::vector<Material>& get_materials() const
    {
        return materials;
    }
    
    const uint32_t get_vertex_count() const
    {
        return static_cast<uint32_t>(model_data.size());
    }
    
    struct VertexBasicAttribs
    {
        float3 pos;
        float3 normal;
        float2 uv0;
        float3 tangent;
    };

    const std::vector<VertexBasicAttribs>& get_vertex_data() const
    {
        return model_data;
    }

    const uint32_t get_index_count() const
    {
        return static_cast<uint32_t>(indices_data.size());
    }

    const std::vector<uint32_t>& get_index_data() const
    {
        return indices_data;
    }

    const float4x4& get_root_transform() const
    {
        return root_transform;
    }

    struct ImageData
    {
        const char8_t* name = nullptr;
        int width = 0;
        int height = 0;
        int num_components = 0;
        int component_size = 0;

        TEXTURE_FORMAT tex_format = TEXTURE_FORMAT::TEX_FORMAT_UNKNOWN;
        TextureLoader::IMAGE_FILE_FORMAT file_format = TextureLoader::IMAGE_FILE_FORMAT::IMAGE_FILE_FORMAT_UNKNOWN;

        const void* pData = nullptr;
        size_t data_size = 0;
    };
    uint32_t add_texture(RenderObject::IRenderDevice* render_device, const ImageData& image);
    
    RenderObject::ITexture* get_texture(uint32_t index) const
    {
        auto& texture_info = textures[index];
        if (texture_info.texture)
        {
            return texture_info.texture;
        }

        return nullptr;
    }
private:
    void load_node(const tinygltf::Model& gltf_model, uint32_t node_index);
    void load_mesh(const tinygltf::Model& gltf_model, uint32_t mesh_index);
    void load_materials(const tinygltf::Model& gltf_model);
    void load_textures(RenderObject::IRenderDevice* render_device, const tinygltf::Model& gltf_model, const std::string& base_dir);

    uint32_t num_texture_attributes = 0;
    const TextureAttributeDesc* texture_attribute_descs = nullptr;

    tinygltf::Model model;

    std::vector<Mesh> meshes; // Vector of meshes loaded from the model file
    std::vector<Material> materials; // Materials used by the mesh

    std::vector<VertexBasicAttribs> model_data;
    std::vector<uint32_t> indices_data; // Indices for indexed drawing

    float4x4 root_transform = float4x4::Identity();
    struct TextureInfo
    {
        RefCntAutoPtr<RenderObject::ITexture> texture = nullptr;

        explicit operator bool() const
        {
            return texture != nullptr;
        }
    };

    std::vector<TextureInfo> textures;
};

CYBER_END_NAMESPACE
CYBER_END_NAMESPACE