#pragma once
#include "core/Core.h"
#include "platform/configure.h"
#include "eastl/vector.h"
#include "math/advanced_math.hpp"
#include "graphics/interface/render_device.hpp"
#include "GLFW/tiny_gltf.h"


CYBER_BEGIN_NAMESPACE(Cyber)
CYBER_BEGIN_NAMESPACE(ModelLoader)
/*
struct Primitive
{
    const uint32_t first_index;
    const uint32_t index_count;
    const uint32_t vertex_count;
    const uint32_t material_index;

    const BoundBox bound_box;

    Primitive(uint32_t _first_index, uint32_t _index_count, uint32_t _vertex_count, uint32_t _material_index, const float3& _min, const float3& _max)
        : first_index(_first_index), index_count(_index_count), vertex_count(_vertex_count), material_index(_material_index), bound_box{ _min, _max } {}

    Primitive(Primitive&&) = default;

    bool has_indices() const
    {
        return index_count > 0;
    }
};


struct Mesh
{
    eastl::string name;
    eastl::vector<Primitive> primitives;
    BoundBox bound_box; // Bounding box of the mesh, calculated from all primitives

    bool is_valid_mesh() const
    {
        return !primitives.empty();
    }
};

struct Node;
struct Skin
{
    eastl::string name;
    const Node* skeleton_root = nullptr;
    eastl::vector<float4x4> inverse_bind_matrices; // Inverse bind matrices for skinning, indexed by joint index
    eastl::vector<const Node*> joints; // Joints in the skin, indexed by joint index
};

struct Camera
{
    eastl::string name;

    enum class Projection
    {
        Unknown,
        Perspective,
        Orthographic
    } type = Projection::Unknown;

    struct perspective_attribs
    {
        float aspect_ratio = 1.0f; // Aspect ratio of the camera
        float yfov = 0.0f; // Vertical field of view in radians
        float znear = 0.1f; // Near clipping plane distance
        float zfar = 1000.0f; // Far clipping plane distance
    };
    struct orthographic_attribs
    {
        float xmag = 1.0f; // Horizontal magnification
        float ymag = 1.0f; // Vertical magnification
        float znear = 0.1f; // Near clipping plane distance
        float zfar = 1000.0f; // Far clipping plane distance
    };

    union
    {
        perspective_attribs perspective;
        orthographic_attribs orthographic;
    };
};

struct Node
{
    // Index in Model.linearNodes array
    const int32_t index;

    int skin_transforms_index = -1; // Index in Model.skins array, -1 if no skin is applied

    eastl::string name; // Node name

    const Node* parent = nullptr; // Parent node, nullptr if root node

    eastl::vector<const Node*> children; // Child nodes

    const Mesh* mesh = nullptr; // Pointer to the mesh associated with this node, nullptr if no mesh
    const Camera* camera = nullptr; // Pointer to the camera associated with this node, nullptr if no camera
    const Skin* skin = nullptr; // Pointer to the skin associated with this node, nullptr if no skin

    float3 translation = { 0.0f, 0.0f, 0.0f }; // Translation of the node in world space
    float4 rotation = { 0.0f, 0.0f, 0.0f, 1.0f }; // Rotation of the node in world space (quaternion)
    float3 scale = { 1.0f, 1.0f, 1.0f }; // Scale of the node in world space
    float4x4 local_transform = float4x4::Identity();

    explicit Node(int32_t _index)
        : index(_index)
    {
    }
};

struct Scene
{
    eastl::string name;
    eastl::vector<const Node*> root_nodes;
    eastl::vector<const Node*> linear_nodes; 
};

struct Model
{
    struct VertexBasicAttribs
    {
        float3 pos;
        float3 normal;
        float2 uv0;
        float2 uv1;
    };

    struct VertexSkinAttribs
    {
        float4 joint0;
        float4 weight0;
    };

    enum VERTEX_BUFFER_ID
    {
        VERTEX_BUFFER_ID_BASIC_ATTRIBS = 0,
        VERTEX_BUFFER_ID_SKIN_ATTRIBS = 1,
    };
};
*/

struct Mesh
{
    struct VertexBasicAttribs
    {
        float3 pos;
        float3 normal;
        float2 uv0;
        float3 tangent;
    };

    std::vector<VertexBasicAttribs> model_data;
    std::vector<uint16_t> indices_data; // Indices for indexed drawing
    std::vector<std::string> image_paths; // Paths to textures used by the mesh
};

struct ModelCreateInfo
{
    const char* file_path = nullptr; // Path to the model file
};

class ModelLoader
{
public:
    ModelLoader( RenderObject::IRenderDevice* render_device, RenderObject::IDeviceContext* context, const ModelCreateInfo& create_info)
    {
        load_from_file(render_device, context, create_info);
    }

    //static tinygltf::Model create_model(const ModelCreateInfo& create_info);

    void load_from_file(RenderObject::IRenderDevice* render_device, RenderObject::IDeviceContext* context, const ModelCreateInfo& create_info);

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

private:
    tinygltf::Model model;
    
    std::vector<Mesh> meshes; // Vector of meshes loaded from the model file
};

CYBER_END_NAMESPACE
CYBER_END_NAMESPACE