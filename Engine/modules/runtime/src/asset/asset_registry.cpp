#include "asset/asset_registry.h"

#include <algorithm>
#include <fstream>
#include <nlohmann/json.hpp>

namespace Cyber
{
    namespace
    {
        constexpr uint32_t kAssetRegistryVersion = 1;

        bool path_starts_with_parent_reference(const std::filesystem::path& path)
        {
            for (const auto& part : path)
            {
                if (part == "..")
                    return true;
            }
            return false;
        }

        std::string guid_to_string(AssetGuid guid)
        {
            return guid.IsValid() ? guid.ToString() : std::string {};
        }

        AssetGuid guid_from_json(const nlohmann::json& value)
        {
            AssetGuid guid;
            if (!value.is_string())
                return guid;
            if (!AssetGuid::FromString(value.get<std::string>(), guid))
                return {};
            return guid;
        }

        nlohmann::json dependencies_to_json(const std::vector<AssetGuid>& dependencies)
        {
            nlohmann::json out = nlohmann::json::array();
            for (AssetGuid dependency : dependencies)
            {
                if (dependency.IsValid())
                    out.push_back(dependency.ToString());
            }
            return out;
        }

        std::vector<AssetGuid> dependencies_from_json(const nlohmann::json& value)
        {
            std::vector<AssetGuid> out;
            if (!value.is_array())
                return out;

            for (const nlohmann::json& item : value)
            {
                AssetGuid guid = guid_from_json(item);
                if (guid.IsValid())
                    out.push_back(guid);
            }
            return out;
        }
    }

    void AssetRegistry::Clear()
    {
        m_records.clear();
    }

    bool AssetRegistry::Load(const std::filesystem::path& path)
    {
        Clear();

        std::ifstream file(path);
        if (!file)
            return false;

        nlohmann::json root;
        try
        {
            file >> root;
        }
        catch (...)
        {
            Clear();
            return false;
        }

        const nlohmann::json* assets = nullptr;
        if (root.is_object() && root.contains("assets"))
            assets = &root["assets"];
        else if (root.is_array())
            assets = &root;

        if (!assets || !assets->is_array())
            return false;

        for (const nlohmann::json& item : *assets)
        {
            if (!item.is_object())
                continue;

            AssetRegistryRecord record;
            record.guid = guid_from_json(item.value("guid", nlohmann::json {}));

            AssetType type = AssetType::Unknown;
            if (!TryParseAssetType(item.value("type", std::string {}), type))
                type = AssetType::Unknown;
            record.type = type;

            record.assetPath = NormalizePath(item.value("assetPath", std::string {}));
            record.displayName = item.value("displayName", std::string {});
            record.sourcePath = NormalizePath(item.value("sourcePath", std::string {}));
            record.sourceHash = item.value("sourceHash", 0ull);
            record.editorAssetHash = item.value("editorAssetHash", 0ull);
            record.cookedAssetHash = item.value("cookedAssetHash", 0ull);
            record.importerVersion = item.value("importerVersion", 0u);
            record.assetFormatVersion = item.value("assetFormatVersion", 0u);
            record.dependencies = dependencies_from_json(item.value("dependencies", nlohmann::json::array()));
            record.thumbnailPath = NormalizePath(item.value("thumbnailPath", std::string {}));
            record.packageName = item.value("packageName", std::string {});
            record.chunkId = item.value("chunkId", 0u);

            if (record.IsValid())
                Upsert(record);
        }

        return true;
    }

    bool AssetRegistry::Save(const std::filesystem::path& path) const
    {
        std::error_code ec;
        const std::filesystem::path parent = path.parent_path();
        if (!parent.empty())
            std::filesystem::create_directories(parent, ec);
        if (ec)
            return false;

        nlohmann::json root;
        root["version"] = kAssetRegistryVersion;
        root["assets"] = nlohmann::json::array();

        for (const AssetRegistryRecord& record : m_records)
        {
            if (!record.IsValid())
                continue;

            nlohmann::json item;
            item["guid"] = guid_to_string(record.guid);
            item["type"] = ToString(record.type);
            item["assetPath"] = NormalizePath(record.assetPath);
            item["displayName"] = record.displayName;
            item["sourcePath"] = NormalizePath(record.sourcePath);
            item["sourceHash"] = record.sourceHash;
            item["editorAssetHash"] = record.editorAssetHash;
            item["cookedAssetHash"] = record.cookedAssetHash;
            item["importerVersion"] = record.importerVersion;
            item["assetFormatVersion"] = record.assetFormatVersion;
            item["dependencies"] = dependencies_to_json(record.dependencies);
            item["thumbnailPath"] = NormalizePath(record.thumbnailPath);
            item["packageName"] = record.packageName;
            item["chunkId"] = record.chunkId;
            root["assets"].push_back(std::move(item));
        }

        std::ofstream file(path, std::ios::trunc);
        if (!file)
            return false;

        file << root.dump(4);
        return file.good();
    }

    void AssetRegistry::Upsert(const AssetRegistryRecord& record)
    {
        if (!record.IsValid())
            return;

        AssetRegistryRecord normalized = record;
        normalized.assetPath = NormalizePath(normalized.assetPath);
        normalized.sourcePath = NormalizePath(normalized.sourcePath);
        normalized.thumbnailPath = NormalizePath(normalized.thumbnailPath);

        auto it = std::find_if(m_records.begin(), m_records.end(),
            [&](const AssetRegistryRecord& existing)
            {
                return existing.guid == normalized.guid ||
                       NormalizePath(existing.assetPath) == normalized.assetPath;
            });

        if (it != m_records.end())
            *it = std::move(normalized);
        else
            m_records.push_back(std::move(normalized));

        std::sort(m_records.begin(), m_records.end(),
            [](const AssetRegistryRecord& lhs, const AssetRegistryRecord& rhs)
            {
                return lhs.assetPath < rhs.assetPath;
            });
    }

    bool AssetRegistry::Remove(AssetGuid guid)
    {
        auto it = std::remove_if(m_records.begin(), m_records.end(),
            [&](const AssetRegistryRecord& record) { return record.guid == guid; });
        const bool removed = it != m_records.end();
        m_records.erase(it, m_records.end());
        return removed;
    }

    const AssetRegistryRecord* AssetRegistry::Find(AssetGuid guid) const
    {
        auto it = std::find_if(m_records.begin(), m_records.end(),
            [&](const AssetRegistryRecord& record) { return record.guid == guid; });
        return it == m_records.end() ? nullptr : &(*it);
    }

    AssetRegistryRecord* AssetRegistry::Find(AssetGuid guid)
    {
        auto it = std::find_if(m_records.begin(), m_records.end(),
            [&](const AssetRegistryRecord& record) { return record.guid == guid; });
        return it == m_records.end() ? nullptr : &(*it);
    }

    const AssetRegistryRecord* AssetRegistry::FindByAssetPath(std::string_view assetPath) const
    {
        const std::string normalized = NormalizePath(assetPath);
        auto it = std::find_if(m_records.begin(), m_records.end(),
            [&](const AssetRegistryRecord& record)
            {
                return NormalizePath(record.assetPath) == normalized;
            });
        return it == m_records.end() ? nullptr : &(*it);
    }

    const AssetRegistryRecord* AssetRegistry::FindBySourcePath(std::string_view sourcePath) const
    {
        const std::string normalized = NormalizePath(sourcePath);
        auto it = std::find_if(m_records.begin(), m_records.end(),
            [&](const AssetRegistryRecord& record)
            {
                return NormalizePath(record.sourcePath) == normalized;
            });
        return it == m_records.end() ? nullptr : &(*it);
    }

    std::string AssetRegistry::NormalizePath(std::string_view path)
    {
        std::string out(path);
        std::replace(out.begin(), out.end(), '\\', '/');

        while (out.size() > 1 && out.back() == '/')
            out.pop_back();

        return out;
    }

    std::string AssetRegistry::MakeStoredPath(const std::filesystem::path& path,
                                              const std::filesystem::path& root)
    {
        std::error_code ec;
        std::filesystem::path normalizedPath = path.lexically_normal();
        if (!root.empty())
        {
            std::filesystem::path normalizedRoot = root.lexically_normal();
            std::filesystem::path relative = std::filesystem::relative(normalizedPath, normalizedRoot, ec);
            if (!ec && !relative.empty() && !path_starts_with_parent_reference(relative))
                return NormalizePath(relative.generic_string());
        }

        return NormalizePath(normalizedPath.generic_string());
    }
}
