#pragma once

#include "asset/asset_importer.h"

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace Cyber
{
    inline constexpr uint32_t kTextureAssetPayloadMagic = MakeAssetFourCC('C', 'T', 'E', 'X');
    inline constexpr uint32_t kTextureAssetPayloadVersion = 1;
    inline constexpr uint32_t kTextureImporterVersion = 1;

    struct TextureAssetPayloadHeader
    {
        uint32_t magic = kTextureAssetPayloadMagic;
        uint32_t version = kTextureAssetPayloadVersion;
        uint32_t width = 0;
        uint32_t height = 0;
        uint32_t sourceExtensionSize = 0;
        uint32_t reserved = 0;
        uint64_t sourceDataOffset = 0;
        uint64_t sourceDataSize = 0;
    };

    struct TextureEditorAssetInfo
    {
        AssetFileHeader fileHeader {};
        TextureAssetPayloadHeader payloadHeader {};
        std::string sourceExtension;
    };

    class CYBER_RUNTIME_API TextureImporter final : public IAssetImporter
    {
    public:
        [[nodiscard]] AssetType Type() const override { return AssetType::Texture; }
        [[nodiscard]] uint32_t Version() const override { return kTextureImporterVersion; }
        [[nodiscard]] bool Import(const AssetImportRequest& request,
                                  AssetImportResult& outResult) const override;

        [[nodiscard]] static bool IsSupportedSourceExtension(std::string_view extension);
        [[nodiscard]] static bool ReadInfo(const std::filesystem::path& path,
                                           TextureEditorAssetInfo& outInfo);

    private:
        [[nodiscard]] static bool ReadSourceBytes(const std::filesystem::path& path,
                                                  std::vector<uint8_t>& outBytes);
    };
}
