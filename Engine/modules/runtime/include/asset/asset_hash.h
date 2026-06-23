#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace Cyber::AssetHash
{
    inline constexpr uint64_t kFnv1a64Offset = 14695981039346656037ull;
    inline constexpr uint64_t kFnv1a64Prime = 1099511628211ull;

    [[nodiscard]] inline uint64_t HashBytes(const void* data, size_t size, uint64_t seed = kFnv1a64Offset)
    {
        const auto* bytes = static_cast<const uint8_t*>(data);
        uint64_t hash = seed;
        for (size_t i = 0; i < size; ++i)
        {
            hash ^= bytes[i];
            hash *= kFnv1a64Prime;
        }
        return hash;
    }

    [[nodiscard]] inline uint64_t HashString(std::string_view text, uint64_t seed = kFnv1a64Offset)
    {
        return HashBytes(text.data(), text.size(), seed);
    }

    [[nodiscard]] inline uint64_t Combine(uint64_t lhs, uint64_t rhs)
    {
        uint64_t value = lhs;
        value ^= rhs + 0x9e3779b97f4a7c15ull + (value << 6) + (value >> 2);
        return value;
    }
}

