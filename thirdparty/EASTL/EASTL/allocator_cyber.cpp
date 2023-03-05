#include "allocator_cyber.h"

#if EASTL_ALLOCATOR_CYBER

#include "../../mimalloc/include/mimalloc.h"

namespace eastl
{
    void* allocator_cyber::allocate(size_t n, int /*flags*/)
    {
        return mi_malloc(n);
    }

	void* allocator_cyber::allocate(size_t n, size_t alignment, size_t alignmentOffset, int /*flags*/)
    {
        return mi_malloc_aligned(n, alignment);
    }

	void allocator_cyber::deallocate(void* p, size_t /*n*/)
    {
        mi_free(p);
    }

    EASTL_API allocator_cyber gDefaultAllocatorCyber;
    EASTL_API allocator_cyber* gpDefaultAllocatorCyber = &gDefaultAllocatorCyber;

    EASTL_API allocator_cyber* GetDefaultAllocatorCyber()
    {
        return gpDefaultAllocatorCyber;
    }
    
	EASTL_API allocator_cyber* SetDefaultAllocatorCyber(allocator_cyber* pAllocator)
    {
        allocator_cyber* const pPrevAllocator = gpDefaultAllocatorCyber;
        gpDefaultAllocatorCyber = pAllocator;
        return pPrevAllocator;
    }
}

#endif