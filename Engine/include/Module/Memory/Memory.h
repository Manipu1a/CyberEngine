#pragma once

#include <mimalloc.h>
#include <mimalloc-new-delete.h>

namespace Cyber
{
    
template<typename T, typename... Args>
static T* cb_placement_new(void* ptr, Args&&... args)
{
    return mi_new (ptr) T(eastl::forward<Args>(args)...);
}

template<typename T, typename... Args>
static T* cb_new_internal(Args&&... args)
{
    void* ptr = mi_malloc_aligned(alignof(T), sizeof(T));
    return cb_placement_new<T>(ptr, eastl::forward<Args>(args)...);
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