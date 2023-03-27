#pragma once

#include "platform/configure.h"
#define XXH_INLINE_ALL
#include "xxhash3/xxhash.h"

namespace Cyber
{
    FORCEINLINE static size_t cyber_hash(const void* buffer, size_t size, size_t seed)
    {
    #if SIZE_MAX == UINT64_MAX
        return XXH64(buffer, size, seed);
    #elif SIZE_MAX == UINT32_MAX
        return XXH32(buffer, size, seed);
    #else
        #error "unsupported hash size!"
    #endif
    }

    FORCEINLINE static size_t cyber_hash64(const void* buffer, size_t size, size_t seed)
    {
        return XXH64(buffer, size, seed);
    }

    FORCEINLINE static size_t cyber_hash32(const void* buffer, size_t size, size_t seed)
    {
        return XXH32(buffer, size, seed);
    }
    

    
}
