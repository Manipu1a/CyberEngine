#pragma once

#include "asset/asset_importer.h"

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace Cyber
{
    inline constexpr uint32_t kMeshAssetPayloadMagic = MakeAssetFourCC('C', 'M', 'E', 'S');
    inline constexpr uint32_t kMeshAssetPayloadVersion = 1;
    inline constexpr uint32_t kMeshImporterVersion = 1;

    struct MeshAssetPayloadHeader
    {
        uint32_t magic = kMeshAssetPayloadMagic;
        uint32_t version = kMeshAssetPayloadVersion;
        uint32_t sourceExtensionSize = 0;
        uint32_t reserved = 0;
        uint64_t sourceDataOffset = 0;
        uint64_t sourceDataSize = 0;
    };

    struct MeshEditorAssetInfo
    {
        AssetFileHeader fileHeader {};
        MeshAssetPayloadHeader payloadHeader {};
        std::string sourceExtension;
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

    private:
        [[nodiscard]] static bool ReadSourceBytes(const std::filesystem::path& path,
                                                  std::vector<uint8_t>& outBytes);
    };
}
