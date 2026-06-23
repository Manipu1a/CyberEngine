#pragma once

#include "cyber_runtime.config.h"

#include <cstdint>
#include <string>
#include <string_view>

namespace Cyber
{
    struct CYBER_RUNTIME_API AssetGuid
    {
        uint64_t high = 0;
        uint64_t low = 0;

        constexpr AssetGuid() = default;
        constexpr AssetGuid(uint64_t highValue, uint64_t lowValue)
            : high(highValue), low(lowValue)
        {
        }

        [[nodiscard]] static AssetGuid Create();
        [[nodiscard]] static bool FromString(std::string_view text, AssetGuid& outGuid);

        [[nodiscard]] std::string ToString() const;
        [[nodiscard]] constexpr bool IsValid() const { return high != 0 || low != 0; }

        [[nodiscard]] constexpr bool operator==(const AssetGuid& rhs) const
        {
            return high == rhs.high && low == rhs.low;
        }

        [[nodiscard]] constexpr bool operator!=(const AssetGuid& rhs) const
        {
            return !(*this == rhs);
        }

        [[nodiscard]] constexpr bool operator<(const AssetGuid& rhs) const
        {
            return high < rhs.high || (high == rhs.high && low < rhs.low);
        }
    };

    inline constexpr AssetGuid kInvalidAssetGuid {};
}

