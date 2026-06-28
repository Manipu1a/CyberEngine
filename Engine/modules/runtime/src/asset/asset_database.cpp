#include "asset/asset_database.h"

namespace Cyber
{
    AssetDatabase::AssetDatabase(std::filesystem::path contentRoot)
    {
        SetContentRoot(std::move(contentRoot));
    }

    void AssetDatabase::SetContentRoot(std::filesystem::path contentRoot)
    {
        m_contentRoot = contentRoot.lexically_normal();
    }

    std::filesystem::path AssetDatabase::RegistryPath() const
    {
        if (m_contentRoot.empty())
            return {};
        return m_contentRoot / "Registry" / "AssetRegistry.json";
    }

    bool AssetDatabase::Load()
    {
        m_registry.Clear();
        const std::filesystem::path path = RegistryPath();
        if (path.empty())
            return false;

        std::error_code ec;
        if (!std::filesystem::exists(path, ec))
            return true;

        return m_registry.Load(path);
    }

    bool AssetDatabase::Save() const
    {
        const std::filesystem::path path = RegistryPath();
        if (path.empty())
            return false;
        return m_registry.Save(path);
    }

    std::string AssetDatabase::MakeStoredPath(const std::filesystem::path& path) const
    {
        return AssetRegistry::MakeStoredPath(path, m_contentRoot);
    }
}
