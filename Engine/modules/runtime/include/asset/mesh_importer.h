#pragma once

#include "asset/asset_importer.h"
#include "asset/cooked_mesh.h"

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace Cyber
{
    inline constexpr uint32_t kMeshAssetPayloadMagic = MakeAssetFourCC('C', 'M', 'E', 'S');
    inline constexpr uint32_t kMeshAssetPayloadVersion = 2;
    inline constexpr uint32_t kMeshImporterVersion = 2;

    struct MeshAssetPayloadHeader
    {
        uint32_t magic = kMeshAssetPayloadMagic;
        uint32_t version = kMeshAssetPayloadVersion;
        uint32_t sourceExtensionSize = 0;
        uint32_t vertexStride = sizeof(CookedMeshVertex);
        uint64_t verticesOffset = 0;
        uint64_t vertexCount = 0;
        uint64_t indicesOffset = 0;
        uint64_t indexCount = 0;
        uint64_t meshesOffset = 0;
        uint64_t meshCount = 0;
        uint64_t primitivesOffset = 0;
        uint64_t primitiveCount = 0;
        uint64_t materialsOffset = 0;
        uint64_t materialCount = 0;
        uint64_t texturesOffset = 0;
        uint64_t textureCount = 0;
        uint64_t textureDataOffset = 0;
        uint64_t textureDataSize = 0;
    };

    struct MeshEditorAssetInfo
    {
        AssetFileHeader fileHeader {};
        MeshAssetPayloadHeader payloadHeader {};
        std::string sourceExtension;
        bool isCooked = false;
    };

    class CYBER_RUNTIME_API MeshImporter final : public IAssetImporter
    {
    public:
        [[nodiscard]] AssetType Type() const override { return AssetType::Mesh; }
        [[nodiscard]] uint32_t Version() const override { return kMeshImporterVersion; }
        [[nodiscard]] bool Import(const AssetImportRequest& request,
                                  AssetImportResult& outResult) const override;

        [[nodiscard]] static bool IsSupportedSourceExtension(std::string_view extension);
        [[nodiscard]] static bool ReadInfo(const std::filesystem::path& path,
                                           MeshEditorAssetInfo& outInfo);
        [[nodiscard]] static bool ReadCookedData(const std::filesystem::path& path,
                                                 CookedMeshData& outData,
                                                 std::string* outError = nullptr);
        [[nodiscard]] static bool ReadEmbeddedSource(const std::filesystem::path& path,
                                                     MeshEditorAssetInfo& outInfo,
                                                     std::vector<uint8_t>& outBytes);
        [[nodiscard]] static bool WriteEmbeddedSourceToCache(const std::filesystem::path& path,
                                                             const std::filesystem::path& cacheRoot,
                                                             std::filesystem::path& outPath);

    private:
        [[nodiscard]] static bool ReadFileBytes(const std::filesystem::path& path,
                                                std::vector<uint8_t>& outBytes);
    };
}
