#pragma once
#include "cyber_game.config.h"
#include "platform/configure.h"
#include "math/basic_math.hpp"
#include "common/smart_ptr.h"

CYBER_BEGIN_NAMESPACE(Cyber)

struct MeshVertex
{
    float3 position;
    float3 normal;
    float3 tangent;
    float2 uv;
};

class CYBER_GAME_API Mesh
{
public:
    Mesh();
    Mesh(uint32_t vertex_count,
         const MeshVertex* vertex_data,
         uint32_t index_count,
         const uint32_t* index_data,
         const float4x4& root_transform);
         
    uint32_t vertex_count;
    RefCntAutoPtr<MeshVertex> vertex_data;
    uint32_t index_count;
    RefCntAutoPtr<uint32_t> index_data;
    float4x4 root_transform;
};
CYBER_END_NAMESPACE