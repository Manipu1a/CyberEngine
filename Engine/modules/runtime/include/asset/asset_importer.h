#pragma once

#include "asset/asset_registry.h"

#include <filesystem>
#include <string>

namespace Cyber
{
    struct AssetImportRequest
    {
        std::filesystem::path sourcePath;
        std::filesystem::path destinationPath;
        std::filesystem::path contentRoot;
        AssetGuid existingGuid {};
        uint32_t width = 0;
        uint32_t height = 0;
    };

    struct AssetImportResult
    {
        AssetRegistryRecord registryRecord {};
        std::string error;

        [[nodiscard]] bool Succeeded() const
        {
            return error.empty() && registryRecord.IsValid();
        }
    };

    class CYBER_RUNTIME_API IAssetImporter
    {
    public:
        virtual ~IAssetImporter() = default;

        [[nodiscard]] virtual AssetType Type() const = 0;
        [[nodiscard]] virtual uint32_t Version() const = 0;
        [[nodiscard]] virtual bool Import(const AssetImportRequest& request,
                                          AssetImportResult& outResult) const = 0;
    };
}
