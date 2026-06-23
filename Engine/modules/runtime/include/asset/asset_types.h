#pragma once

#include "asset/asset_guid.h"
#include "cyber_runtime.config.h"

#include <cstdint>
#include <string_view>

namespace Cyber
{
    [[nodiscard]] constexpr uint32_t MakeAssetFourCC(char a, char b, char c, char d)
    {
        return (static_cast<uint32_t>(static_cast<uint8_t>(a)) << 0) |
               (static_cast<uint32_t>(static_cast<uint8_t>(b)) << 8) |
               (static_cast<uint32_t>(static_cast<uint8_t>(c)) << 16) |
               (static_cast<uint32_t>(static_cast<uint8_t>(d)) << 24);
    }

    enum class AssetType : uint32_t
    {
        Unknown = 0,
        Texture,
        Mesh,
        Material,
        Scene,
        Animation,
        Skeleton,
        Audio,
        Shader,
        Prefab,
        Font,
        RenderTarget,
        PhysicsCollision,
    };

    inline constexpr uint32_t kAnyAssetPlatform = 0;
    inline constexpr uint32_t kWindowsD3D12AssetPlatform = MakeAssetFourCC('W', 'D', '1', '2');

    inline constexpr uint32_t kAssetFileMagic = MakeAssetFourCC('C', 'A', 'S', 'T');
    inline constexpr uint32_t kAssetFileFormatVersion = 1;

    struct AssetFileHeader
    {
        uint32_t magic = kAssetFileMagic;
        uint32_t headerSize = sizeof(AssetFileHeader);
        uint32_t formatVersion = kAssetFileFormatVersion;
        AssetType assetType = AssetType::Unknown;
        AssetGuid assetGuid {};
        uint64_t contentHash = 0;
        uint64_t dependencyHash = 0;
        uint32_t cookerVersion = 0;
        uint32_t platformTag = kAnyAssetPlatform;
        uint64_t payloadOffset = 0;
        uint64_t payloadSize = 0;

        [[nodiscard]] bool IsValid() const
        {
            return magic == kAssetFileMagic &&
                   headerSize >= sizeof(AssetFileHeader) &&
                   formatVersion > 0 &&
                   assetType != AssetType::Unknown &&
                   assetGuid.IsValid();
        }

        [[nodiscard]] bool IsCompatible(AssetType expectedType,
                                         uint32_t maxSupportedVersion,
                                         uint32_t expectedPlatform = kAnyAssetPlatform) const
        {
            const bool platformMatches = expectedPlatform == kAnyAssetPlatform ||
                                         platformTag == kAnyAssetPlatform ||
                                         platformTag == expectedPlatform;
            return IsValid() &&
                   assetType == expectedType &&
                   formatVersion <= maxSupportedVersion &&
                   platformMatches;
        }
    };

    static_assert(sizeof(AssetGuid) == 16, "AssetGuid must stay binary stable.");

    [[nodiscard]] CYBER_RUNTIME_API const char* ToString(AssetType type);
    [[nodiscard]] CYBER_RUNTIME_API bool TryParseAssetType(std::string_view text, AssetType& outType);
}

