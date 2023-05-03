#pragma once

#include "configure.h"
#include <EASTL/internal/function.h>
#include "cyber_runtime.config.h"

namespace Cyber
{
#define CYBER_KB (1024)
#define CYBER_MB (CYBER_KB * 1024)
#define CYBER_GB (CYBER_MB * 1024)

CYBER_RUNTIME_EXTERN_C CYBER_RUNTIME_EXTERN_C void* _cyber_malloc(size_t size);
CYBER_RUNTIME_EXTERN_C CYBER_RUNTIME_EXTERN_C void* _cyber_calloc(size_t count, size_t size);
CYBER_RUNTIME_EXTERN_C CYBER_RUNTIME_EXTERN_C void* _cyber_malloc_aligned(size_t size, size_t align);
CYBER_RUNTIME_EXTERN_C CYBER_RUNTIME_EXTERN_C void* _cyber_calloc_aligned(size_t count, size_t size, size_t align);
CYBER_RUNTIME_EXTERN_C CYBER_RUNTIME_EXTERN_C void* _cyber_new_n(size_t count, size_t size);
CYBER_RUNTIME_EXTERN_C CYBER_RUNTIME_EXTERN_C void* _cyber_new_aligned(size_t size, size_t align);
CYBER_RUNTIME_EXTERN_C CYBER_RUNTIME_EXTERN_C void _cyber_free(void* ptr) CYBER_NOEXCEPT;
CYBER_RUNTIME_EXTERN_C CYBER_RUNTIME_EXTERN_C void _cyber_free_aligned(void* ptr, size_t alignment);
CYBER_RUNTIME_EXTERN_C CYBER_RUNTIME_EXTERN_C void* _cyber_realloc(void* ptr, size_t size);

template<typename T, typename... Args>
static T* cyber_placement_new(void* ptr, Args&&... args)
{
    return new (ptr) T(eastl::forward<Args>(args)...);
}

template<typename T, typename... Args>
static T* cyber_new(Args&&... args)
{
    void* ptr = _cyber_malloc_aligned(alignof(T), sizeof(T));
    return cyber_placement_new<T>(ptr, eastl::forward<Args>(args)...);
}

#ifndef cyber_new_n
#define cyber_new_n(count, size) _cyber_new_n(count , size)
#endif

#ifndef cyber_malloc
#define cyber_malloc(size) _cyber_malloc(size)
#endif

#ifndef cyber_memalign
#define cyber_memalign(align, size) _cyber_malloc_aligned(align, size)
#endif

#ifndef cyber_calloc
#define cyber_calloc(count, size) _cyber_calloc(count, size)
#endif

#ifndef cyber_calloc_memalign
#define cyber_calloc_memalign(count, align, size) _cyber_calloc_aligned(count, align, size)
#endif

#ifndef cyber_realloc
#define cyber_realloc(ptr, size) _cyber_realloc(ptr, size)
#endif

#ifndef cyber_free
#define cyber_free(ptr) _cyber_free(ptr)
#endif

template <typename T>
void cyber_delete(T* ptr) CYBER_NOEXCEPT
{
    if (ptr)
    {
        ptr->~T();
        _cyber_free_aligned(ptr, alignof(T));
    }
}

template <typename T>
void cyber_free_container(T& container) CYBER_NOEXCEPT
{
    for (auto& it : container)
    {
        cyber_delete(it);
    }
    container.clear();
}

template <typename T>
void cyber_free_map(T& container) CYBER_NOEXCEPT
{
    for (auto& it : container)
    {
        cyber_delete(it.second);
    }
    container.clear();
}

template<class T>
struct cyber_stl_allocator
{
    typedef T value_type;
    typedef std::size_t size_type;
    typedef std::ptrdiff_t  difference_type;
    typedef value_type& reference;
    typedef value_type const& const_reference;
    typedef value_type* pointer;
    typedef value_type const* const_pointer;
    template <class U>
    struct rebind{ typedef cyber_stl_allocator<U> other; };

    cyber_stl_allocator() CYBER_NOEXCEPT = default;
    cyber_stl_allocator(cyber_stl_allocator const&) CYBER_NOEXCEPT = default;
    template <class U>
    cyber_stl_allocator(cyber_stl_allocator<U> const&) CYBER_NOEXCEPT {}
    cyber_stl_allocator select_on_container_copy_construction() const CYBER_NOEXCEPT { return *this; }
    void deallocate(T* ptr, std::size_t) CYBER_NOEXCEPT { cyber_free(ptr); }

#if (__cplusplus >= 201703L)
    [[nodiscard]] T* allocate(size_type count) CYBER_NOEXCEPT
    {
        return static_cast<pointer>(cyber_new_n(count , sizeof(T)));
    }
    [[nodiscard]] T* allocate(size_type count, const void*)
    {
        return allocate(count);
    }
#else
    [[nodiscard]] pointer allocate(size_type count, const void* = 0) CYBER_NOEXCEPT
    {
        return static_cast<pointer>(cyber_new_n(count , sizeof(T)));
    }
#endif

#if (__cplusplus >= 201103L) || (_MSC_VER >= 1900)
    using propagate_on_container_copy_assignment = std::true_type;
    using propagate_on_container_move_assignment = std::true_type;
    using propagate_on_container_swap = std::true_type;
    using is_always_equal = std::true_type;
    template <class U, class... Args>
    void construct(U* ptr, Args&&... args)
    {
        cyber_placement_new<U>(ptr, eastl::forward<Args>(args)...);
    }
    template <class U>
    void destroy(U* ptr) CYBER_NOEXCEPT
    {
        ptr->~U();
    }
#else
    void construct(pointer ptr, const_reference value)
    {
        cyber_placement_new<value_type>(ptr, value);
    }
    void destroy(pointer ptr) CYBER_NOEXCEPT
    {
        ptr->~value_type();
    }
#endif

    size_type max_size() const CYBER_NOEXCEPT
    {
        return (PTRDIFF_MAX / sizeof(value_type));
    }
    pointer address(reference value) const CYBER_NOEXCEPT
    {
        return &value;
    }
    const_pointer address(const_reference value) const CYBER_NOEXCEPT
    {
        return &value;
    }
};

template<class T1, class T2> bool operator==(const cyber_stl_allocator<T1>&, const cyber_stl_allocator<T2>&) CYBER_NOEXCEPT
{
    return true;
}
template<class T1,class T2> bool operator!=(const cyber_stl_allocator<T1>&, const cyber_stl_allocator<T2>&) CYBER_NOEXCEPT
{
    return false;
}

}