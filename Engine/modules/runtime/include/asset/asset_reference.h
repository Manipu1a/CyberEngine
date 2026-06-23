#pragma once

#include "asset/asset_guid.h"
#include "asset/asset_types.h"

#include <string>
#include <string_view>

namespace Cyber
{
    struct AssetId
    {
        AssetGuid guid {};

        constexpr AssetId() = default;
        constexpr explicit AssetId(AssetGuid value)
            : guid(value)
        {
        }

        [[nodiscard]] constexpr bool IsValid() const { return guid.IsValid(); }
        [[nodiscard]] std::string ToString() const { return guid.ToString(); }

        [[nodiscard]] static bool FromString(std::string_view text, AssetId& outId)
        {
            AssetGuid parsed;
            if (!AssetGuid::FromString(text, parsed))
                return false;
            outId = AssetId(parsed);
            return true;
        }

        [[nodiscard]] constexpr bool operator==(const AssetId& rhs) const { return guid == rhs.guid; }
        [[nodiscard]] constexpr bool operator!=(const AssetId& rhs) const { return !(*this == rhs); }
        [[nodiscard]] constexpr bool operator<(const AssetId& rhs) const { return guid < rhs.guid; }
    };

    struct SoftAssetRef
    {
        AssetId id {};
        AssetType expectedType = AssetType::Unknown;

        constexpr SoftAssetRef() = default;
        constexpr SoftAssetRef(AssetId assetId, AssetType type)
            : id(assetId), expectedType(type)
        {
        }

        [[nodiscard]] constexpr bool IsValid() const
        {
            return id.IsValid() && expectedType != AssetType::Unknown;
        }
    };
}

