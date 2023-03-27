#pragma once
#include "platform/configure.h"
#include "utils/hash.h"

#ifndef CYBER_RHI_EXPORT
    #if defined (_MSC_VER)
        #define CYBER_RHI_EXPORT __declspec(dllexport)
    #else
        #define CYBER_RHI_EXPORT
    #endif
#endif

#ifndef CYBER_RHI_API
    #define CYBER_RHI_API CYBER_RHI_EXPORT
#endif

#if defined (_WINDOWS)
    #include "backend/d3d12/d3d12.config.h"
#else

#endif

#if UINTPTR_MAX == UINT32_MAX
    #define RHI_NAME_HASH_SEED 1610612741
#else
    #define RHI_NAME_HASH_SEED 8053064571610612741
#endif

#define rhi_hash(buffer, size, seed) cyber_hash(buffer, size, seed)
#define rhi_name_hash(buffer, size) cyber_hash(buffer, size, RHI_NAME_HASH_SEED)