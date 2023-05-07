#pragma once

#define DECLARE_ZERO(type, var) type var = {};


#if defined (_MSC_VER) && !defined (__clang__)
    #ifndef FORCEINLINE
        #define FORCEINLINE __forceinline
    #endif
    #define DEFINE_ALIGNED(def, a) __declspec(align(a)) def
#else
    #ifndef FORCEINLINE
        #define FORCEINLINE inline __attribute__((always_inline))
    #endif
        #define DEFINE_ALIGNED(def, a) __attribute__((aligned(a))) def
#endif

#ifdef __cplusplus
#define CYBER_EXTERN_C extern "C"
#define CYBER_CONSTEXPR constexpr
#else
#define CYBER_EXTERN_C
#define CYBER_CONSTEXPR
#endif
#ifdef __cplusplus
#define CYBER_UTF8(str) u8##str
#endif
#ifdef __cplusplus
#define CYBER_NOEXCEPT noexcept
#else
#define CYBER_NOEXCEPT
#endif

#ifdef __cplusplus
#define RUNTIME_EXTERN_C extern "C"
#else
#define RUNTIME_EXTERN_C
#endif