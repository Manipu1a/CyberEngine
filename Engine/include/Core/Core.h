#pragma once

#include <EASTL/shared_ptr.h>
#include <EASTL/scoped_ptr.h>
#include <EASTL/unique_ptr.h>

#include <functional>

#ifdef _WIN32
    #ifdef _WIN64
        /* Windows x64 */
        #define CB_PLATFORM_WINDOWS
    #else
		#error "x86 Builds are not supported!"
    #endif
#endif

#ifdef CB_DEBUG
    #define CB_ENABLE_ASSERTS
#endif

#ifdef CB_ENABLE_ASSERTS
    #define CB_ASSERTS(X, ...) { if(!(x)) {CB_ERROR("")} }
#else

#endif

#define CB_BIND_EVENT_FN(fn) std::bind(&fn, this, std::placeholders::_1)

#define BIT(x) (1 << x)

namespace Cyber
{
    template<typename T>
    using Ref = eastl::shared_ptr<T>;
    template<typename T, typename ... Args>
    constexpr Ref<T> CreateRef(Args&& ... args)
    {
        return eastl::make_shared<T>(eastl::forward<Args>(args)...);
    }

    template<typename T>
    using Scope = eastl::unique_ptr<T>;
    template<typename T, typename ... Args>
    constexpr Scope<T> CreateScope(Args&& ... args)
    {
        return eastl::make_unique<T>(eastl::forward<Args>(args)...);
    }

}