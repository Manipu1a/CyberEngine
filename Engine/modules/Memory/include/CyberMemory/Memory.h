#pragma once

#include <mimalloc.h>
#include <mimalloc-new-delete.h>
#include <EASTL/internal/function.h>

namespace Cyber
{
#define CYBER_KB (1024)
#define CYBER_MB (CYBER_KB * 1024)
#define CYBER_GB (CYBER_MB * 1024)

template<typename T, typename... Args>
static T* cyber_placement_new(void* ptr, Args&&... args)
{
    return new (ptr) T(eastl::forward<Args>(args)...);
}

template<typename T, typename... Args>
static T* cyber_new(Args&&... args)
{
    void* ptr = mi_malloc_aligned(alignof(T), sizeof(T));
    return cyber_placement_new<T>(ptr, eastl::forward<Args>(args)...);
}

#ifndef cb_malloc
#define cb_malloc(size) mi_malloc(size)
#endif

#ifndef cb_memalign
#define cb_memalign(align, size) mi_malloc_aligned(align, size)
#endif

#ifndef cb_calloc
#define cb_calloc(count, size) mi_calloc(count, size)
#endif

#ifndef cb_calloc_memalign
#define cb_calloc_memalign(count, align, size) mi_calloc_aligned(count, align, size)
#endif

#ifndef cb_realloc
#define cb_realloc(ptr, size) mi_realloc(ptr, size)
#endif

#ifndef cb_free
#define cb_free(ptr) free(ptr)
#endif

}