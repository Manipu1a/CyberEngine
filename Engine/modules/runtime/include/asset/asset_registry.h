#pragma once

#include "asset/asset_guid.h"
#include "asset/asset_types.h"
#include "cyber_runtime.config.h"

#include <filesystem>
#include <string>
#include <vector>

namespace Cyber
{
    struct AssetRegistryRecord
    {
        AssetGuid guid {};
        AssetType type = AssetType::Unknown;
        std::string assetPath;
        std::string displayName;
        std::string sourcePath;
        uint64_t sourceHash = 0;
        uint64_t editorAssetHash = 0;
        uint64_t cookedAssetHash = 0;
        uint32_t importerVersion = 0;
        uint32_t assetFormatVersion = 0;
        std::vector<AssetGuid> dependencies;
        std::string thumbnailPath;
        std::string packageName;
        uint32_t chunkId = 0;

        [[nodiscard]] bool IsValid() const
        {
            return guid.IsValid() && type != AssetType::Unknown && !assetPath.empty();
        }
    };

    class CYBER_RUNTIME_API AssetRegistry
    {
    public:
        void Clear();

        [[nodiscard]] bool Load(const std::filesystem::path& path);
        [[nodiscard]] bool Save(const std::filesystem::path& path) const;

        void Upsert(const AssetRegistryRecord& record);
        [[nodiscard]] bool Remove(AssetGuid guid);

        [[nodiscard]] const AssetRegistryRecord* Find(AssetGuid guid) const;
        [[nodiscard]] AssetRegistryRecord* Find(AssetGuid guid);
        [[nodiscard]] const AssetRegistryRecord* FindByAssetPath(std::string_view assetPath) const;
        [[nodiscard]] const AssetRegistryRecord* FindBySourcePath(std::string_view sourcePath) const;

        [[nodiscard]] const std::vector<AssetRegistryRecord>& Records() const { return m_records; }

        [[nodiscard]] static std::string NormalizePath(std::string_view path);
        [[nodiscard]] static std::string MakeStoredPath(const std::filesystem::path& path,
                                                        const std::filesystem::path& root = {});

    private:
        std::vector<AssetRegistryRecord> m_records;
    };
}
