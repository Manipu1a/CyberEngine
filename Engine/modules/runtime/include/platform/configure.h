#pragma once

#include "flag_enums.h"

#define DECLARE_ZERO(type, var) type var = {};


#if defined (_MSC_VER) && !defined (__clang__)
    #ifndef CYBER_FORCE_INLINE
        #define CYBER_FORCE_INLINE __forceinline
    #endif
    #define DEFINE_ALIGNED(def, a) __declspec(align(a)) def
#else
    #ifndef CYBER_FORCE_INLINE
        #define CYBER_FORCE_INLINE inline __attribute__((always_inline))
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

#define CYBER_TYPED_ENUM(EnumName, EnumType) \
    enum EnumName : EnumType

#define CYBER_BEGIN_NAMESPACE(Name) \
    namespace Name                  \
    {                               
#define CYBER_END_NAMESPACE(Name)   \
    }

#define CB_BIND_EVENT_FN(fn) std::bind(&fn, this, std::placeholders::_1)

#define BIT(x) (1 << x)

#define IID_ARGS IID_PPV_ARGS


