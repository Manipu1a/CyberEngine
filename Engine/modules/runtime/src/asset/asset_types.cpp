#include "asset/asset_types.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <string>

namespace Cyber
{
    namespace
    {
        struct AssetTypeName
        {
            AssetType type;
            const char* name;
        };

        constexpr std::array<AssetTypeName, 13> kAssetTypeNames {{
            { AssetType::Unknown, "Unknown" },
            { AssetType::Texture, "Texture" },
            { AssetType::Mesh, "Mesh" },
            { AssetType::Material, "Material" },
            { AssetType::Scene, "Scene" },
            { AssetType::Animation, "Animation" },
            { AssetType::Skeleton, "Skeleton" },
            { AssetType::Audio, "Audio" },
            { AssetType::Shader, "Shader" },
            { AssetType::Prefab, "Prefab" },
            { AssetType::Font, "Font" },
            { AssetType::RenderTarget, "RenderTarget" },
            { AssetType::PhysicsCollision, "PhysicsCollision" },
        }};

        bool equals_ignore_case(std::string_view lhs, std::string_view rhs)
        {
            if (lhs.size() != rhs.size())
                return false;

            for (size_t i = 0; i < lhs.size(); ++i)
            {
                const auto a = static_cast<unsigned char>(lhs[i]);
                const auto b = static_cast<unsigned char>(rhs[i]);
                if (std::tolower(a) != std::tolower(b))
                    return false;
            }

            return true;
        }
    }

    const char* ToString(AssetType type)
    {
        for (const AssetTypeName& entry : kAssetTypeNames)
        {
            if (entry.type == type)
                return entry.name;
        }
        return "Unknown";
    }

    bool TryParseAssetType(std::string_view text, AssetType& outType)
    {
        for (const AssetTypeName& entry : kAssetTypeNames)
        {
            if (equals_ignore_case(text, entry.name))
            {
                outType = entry.type;
                return true;
            }
        }

        outType = AssetType::Unknown;
        return false;
    }
}

