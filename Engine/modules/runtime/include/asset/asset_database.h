#pragma once

#include "asset/asset_registry.h"

namespace Cyber
{
    class CYBER_RUNTIME_API AssetDatabase
    {
    public:
        AssetDatabase() = default;
        explicit AssetDatabase(std::filesystem::path contentRoot);

        void SetContentRoot(std::filesystem::path contentRoot);

        [[nodiscard]] const std::filesystem::path& ContentRoot() const { return m_contentRoot; }
        [[nodiscard]] std::filesystem::path RegistryPath() const;

        [[nodiscard]] bool Load();
        [[nodiscard]] bool Save() const;

        [[nodiscard]] AssetRegistry& Registry() { return m_registry; }
        [[nodiscard]] const AssetRegistry& Registry() const { return m_registry; }

        [[nodiscard]] std::string MakeStoredPath(const std::filesystem::path& path) const;

    private:
        std::filesystem::path m_contentRoot;
        AssetRegistry m_registry;
    };
}
