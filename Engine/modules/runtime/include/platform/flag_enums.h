#pragma once
#include <type_traits>

template <typename EnumType>
using _UNDERLYING_ENUM_T = typename std::underlying_type<EnumType>::type;

#define DEFINE_FLAG_ENUM_OPERATORS(ENUMTYPE) \
    extern "C++" \
    {               \
        CYBER_FORCE_INLINE ENUMTYPE& operator|=(ENUMTYPE& a, ENUMTYPE b) { return reinterpret_cast<ENUMTYPE&>(reinterpret_cast<_UNDERLYING_ENUM_T<ENUMTYPE>&>(a) |= static_cast<_UNDERLYING_ENUM_T<ENUMTYPE>>(b)); } \
        CYBER_FORCE_INLINE ENUMTYPE& operator&=(ENUMTYPE& a, ENUMTYPE b) { return reinterpret_cast<ENUMTYPE&>(reinterpret_cast<_UNDERLYING_ENUM_T<ENUMTYPE>&>(a) &= static_cast<_UNDERLYING_ENUM_T<ENUMTYPE>>(b)); } \
        CYBER_FORCE_INLINE ENUMTYPE& operator^=(ENUMTYPE& a, ENUMTYPE b) { return reinterpret_cast<ENUMTYPE&>(reinterpret_cast<_UNDERLYING_ENUM_T<ENUMTYPE>&>(a) ^= static_cast<_UNDERLYING_ENUM_T<ENUMTYPE>>(b)); } \
        CYBER_FORCE_INLINE constexpr ENUMTYPE operator|(ENUMTYPE a, ENUMTYPE b) { return static_cast<ENUMTYPE>(static_cast<_UNDERLYING_ENUM_T<ENUMTYPE>>(a) | static_cast<_UNDERLYING_ENUM_T<ENUMTYPE>>(b)); } \
        CYBER_FORCE_INLINE constexpr ENUMTYPE operator&(ENUMTYPE a, ENUMTYPE b) { return static_cast<ENUMTYPE>(static_cast<_UNDERLYING_ENUM_T<ENUMTYPE>>(a) & static_cast<_UNDERLYING_ENUM_T<ENUMTYPE>>(b)); } \
        CYBER_FORCE_INLINE constexpr ENUMTYPE operator^(ENUMTYPE a, ENUMTYPE b) { return static_cast<ENUMTYPE>(static_cast<_UNDERLYING_ENUM_T<ENUMTYPE>>(a) ^ static_cast<_UNDERLYING_ENUM_T<ENUMTYPE>>(b)); } \
        CYBER_FORCE_INLINE constexpr ENUMTYPE operator~(ENUMTYPE a) { return static_cast<ENUMTYPE>(~static_cast<_UNDERLYING_ENUM_T<ENUMTYPE>>(a)); } \
    }
