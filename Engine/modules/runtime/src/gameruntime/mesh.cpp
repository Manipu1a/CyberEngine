#include "gameruntime/mesh.h"
#include <cstring>

CYBER_BEGIN_NAMESPACE(Cyber)
Mesh::Mesh()
    : vertex_count(0),
      vertex_data(nullptr),
      index_count(0),
      index_data(nullptr),
      root_transform(float4x4::Identity())
{
}

Mesh::Mesh(uint32_t vertex_count,
           const MeshVertex* vertex_data,
           uint32_t index_count,
           const uint32_t* index_data,
           const float4x4& root_transform)
{
    this->vertex_count = vertex_count;
    if (vertex_count > 0 && vertex_data != nullptr)
    {
        this->vertex_data = make_ref_counted_array<MeshVertex>(vertex_count);
        std::memcpy(this->vertex_data.get(), vertex_data, vertex_count * sizeof(MeshVertex));
    }
    else
    {
        this->vertex_data = nullptr;
    }

    this->index_count = index_count;
    if (index_count > 0 && index_data != nullptr)
    {
        this->index_data = make_ref_counted_array<uint32_t>(index_count);
        std::memcpy(this->index_data.get(), index_data, index_count * sizeof(uint32_t));
    }
    else
    {
        this->index_data = nullptr;
    }

    this->root_transform = root_transform;
}

CYBER_END_NAMESPACE