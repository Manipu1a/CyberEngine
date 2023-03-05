#ifndef EASTL_ALLOCATOR_CYBER_H
#define EASTL_ALLOCATOR_CYBER_H

#include "internal/config.h"

#if EASTL_ALLOCATOR_CYBER

namespace eastl
{
    class allocator_cyber
    {
    public:
        allocator_cyber(const char* = NULL) {}

        allocator_cyber(const allocator_cyber&) {}

        allocator_cyber(const allocator_cyber&, const char*) {}

        allocator_cyber& operator=(const allocator_cyber&) { return *this;}

        bool operator==(const allocator_cyber&) { return true; }

		bool operator!=(const allocator_cyber&) { return false; }

        void* allocate(size_t n, int /*flags*/ = 0);

		void* allocate(size_t n, size_t alignment, size_t alignmentOffset, int /*flags*/ = 0);

		void deallocate(void* p, size_t /*n*/);

		const char* get_name() const { return "allocator_cyber"; }

		void set_name(const char*) {}
    };

    inline bool operator==(const allocator_cyber&, const allocator_cyber&) { return true; }
	inline bool operator!=(const allocator_cyber&, const allocator_cyber&) { return false; }

	EASTL_API allocator_cyber* GetDefaultAllocatorCyber();
	EASTL_API allocator_cyber* SetDefaultAllocatorCyber(allocator_cyber* pAllocator);
}

#endif
#endif