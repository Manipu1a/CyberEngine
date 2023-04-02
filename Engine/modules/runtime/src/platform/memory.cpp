#include "platform\memory.h"
#include <mimalloc.h>
#include <mimalloc-new-delete.h>
namespace Cyber 
{
    void* _cyber_malloc(size_t size)
    {
        return mi_malloc(size);
    }
    void* _cyber_calloc(size_t count, size_t size)
    {
        return mi_calloc(count, size);
    }
    void* _cyber_malloc_aligned(size_t size, size_t align)
    {
        return mi_malloc_aligned(size, align);
    }
    void* _cyber_calloc_aligned(size_t count, size_t size, size_t align)
    {
        return mi_calloc_aligned(count, size, align);
    }
    void* _cyber_new_n(size_t count, size_t size)
    {
        return mi_malloc(count * size);
    }
    void* _cyber_new_aligned(size_t size, size_t align)
    {
        return mi_malloc_aligned(align, size);
    }
    void _cyber_free(void* ptr) CYBER_NOEXCEPT
    {
        mi_free(ptr);
    }
    void _cyber_free_aligned(void* ptr, size_t alignment)
    {
        mi_free_aligned(ptr, alignment);
    }
    void* _cyber_realloc(void* ptr, size_t size)
    {
        return mi_realloc(ptr, size);
    }
}