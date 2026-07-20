#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

#if defined(_MSC_VER)
    #if defined(CYBER_COOKED_MESH_EXPORTS)
        #define CYBER_COOKED_MESH_API __declspec(dllexport)
    #else
        #define CYBER_COOKED_MESH_API __declspec(dllimport)
    #endif
#else
    #define CYBER_COOKED_MESH_API
#endif

namespace Cyber
{
    struct CookedMeshVertex
    {
        float position[3] {};
        float normal[3] {};
        float uv0[2] {};
        float tangent[3] {};
    };

    struct CookedMeshPrimitive
    {
        uint32_t firstIndex = 0;
        uint32_t indexCount = 0;
        uint32_t vertexCount = 0;
        uint32_t materialIndex = 0;
        float boundsMin[3] {};
        float boundsMax[3] {};
    };

    struct CookedMeshRecord
    {
        uint32_t firstPrimitive = 0;
        uint32_t primitiveCount = 0;
        char name[64] {};
    };

    struct CookedMeshMaterial
    {
        float baseColorFactor[4] { 1.0f, 1.0f, 1.0f, 1.0f };
        float emissiveFactor[4] { 0.0f, 0.0f, 0.0f, 1.0f };
        float metallicFactor = 1.0f;
        float roughnessFactor = 1.0f;
        int32_t textureIds[5] { -1, -1, -1, -1, -1 };
    };

    struct CookedMeshTextureRecord
    {
        uint32_t width = 0;
        uint32_t height = 0;
        uint32_t componentCount = 0;
        uint32_t componentSize = 0;
        uint64_t dataOffset = 0;
        uint64_t dataSize = 0;
        char name[128] {};
    };

    struct CookedMeshTexture
    {
        CookedMeshTextureRecord record {};
        std::vector<uint8_t> bytes;
    };

    struct CookedMeshData
    {
        std::vector<CookedMeshVertex> vertices;
        std::vector<uint32_t> indices;
        std::vector<CookedMeshRecord> meshes;
        std::vector<CookedMeshPrimitive> primitives;
        std::vector<CookedMeshMaterial> materials;
        std::vector<CookedMeshTexture> textures;
    };

    [[nodiscard]] CYBER_COOKED_MESH_API bool ReadCookedMeshAsset(
        const std::filesystem::path& path, CookedMeshData& outData,
        std::string* outError = nullptr);
}

