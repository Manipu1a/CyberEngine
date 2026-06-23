#include "asset/asset_guid.h"

#include <array>
#include <cctype>
#include <random>

namespace Cyber
{
    namespace
    {
        int hex_value(char c)
        {
            if (c >= '0' && c <= '9')
                return c - '0';
            if (c >= 'a' && c <= 'f')
                return c - 'a' + 10;
            if (c >= 'A' && c <= 'F')
                return c - 'A' + 10;
            return -1;
        }

        void write_u64_be(uint64_t value, uint8_t* outBytes)
        {
            for (int i = 7; i >= 0; --i)
            {
                outBytes[i] = static_cast<uint8_t>(value & 0xffu);
                value >>= 8;
            }
        }

        uint64_t read_u64_be(const uint8_t* bytes)
        {
            uint64_t value = 0;
            for (int i = 0; i < 8; ++i)
                value = (value << 8) | bytes[i];
            return value;
        }

        std::array<uint8_t, 16> to_bytes(const AssetGuid& guid)
        {
            std::array<uint8_t, 16> bytes {};
            write_u64_be(guid.high, bytes.data());
            write_u64_be(guid.low, bytes.data() + 8);
            return bytes;
        }

        AssetGuid from_bytes(const uint8_t* bytes)
        {
            return AssetGuid(read_u64_be(bytes), read_u64_be(bytes + 8));
        }
    }

    AssetGuid AssetGuid::Create()
    {
        std::random_device randomDevice;
        std::array<uint32_t, 8> seedData {
            randomDevice(), randomDevice(), randomDevice(), randomDevice(),
            randomDevice(), randomDevice(), randomDevice(), randomDevice()
        };
        std::seed_seq seed(seedData.begin(), seedData.end());
        std::mt19937_64 generator(seed);

        AssetGuid guid(generator(), generator());
        std::array<uint8_t, 16> bytes = to_bytes(guid);

        bytes[6] = static_cast<uint8_t>((bytes[6] & 0x0fu) | 0x40u);
        bytes[8] = static_cast<uint8_t>((bytes[8] & 0x3fu) | 0x80u);

        return from_bytes(bytes.data());
    }

    bool AssetGuid::FromString(std::string_view text, AssetGuid& outGuid)
    {
        if (text.size() >= 2 && text.front() == '{' && text.back() == '}')
            text = text.substr(1, text.size() - 2);

        std::array<char, 32> digits {};
        size_t digitCount = 0;
        for (char c : text)
        {
            if (c == '-')
                continue;

            if (hex_value(c) < 0 || digitCount >= digits.size())
                return false;

            digits[digitCount++] = c;
        }

        if (digitCount != digits.size())
            return false;

        std::array<uint8_t, 16> bytes {};
        for (size_t i = 0; i < bytes.size(); ++i)
        {
            const int highNibble = hex_value(digits[i * 2]);
            const int lowNibble = hex_value(digits[i * 2 + 1]);
            if (highNibble < 0 || lowNibble < 0)
                return false;
            bytes[i] = static_cast<uint8_t>((highNibble << 4) | lowNibble);
        }

        outGuid = from_bytes(bytes.data());
        return true;
    }

    std::string AssetGuid::ToString() const
    {
        static constexpr char kHex[] = "0123456789abcdef";

        std::array<uint8_t, 16> bytes = to_bytes(*this);
        std::string out;
        out.reserve(36);

        for (size_t i = 0; i < bytes.size(); ++i)
        {
            if (i == 4 || i == 6 || i == 8 || i == 10)
                out.push_back('-');

            out.push_back(kHex[(bytes[i] >> 4) & 0x0f]);
            out.push_back(kHex[bytes[i] & 0x0f]);
        }

        return out;
    }
}

