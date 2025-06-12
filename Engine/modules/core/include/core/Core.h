#pragma once
#include <EASTL/string.h>
#include <EASTL/shared_ptr.h>
#include <EASTL/unique_ptr.h>
#include "common/basic_math.hpp"
#include "debug.h"
#include <functional>
#include "../../../config.h"

#ifdef _WIN32
    #ifdef _WIN64
        /* Windows x64 */
        #define CB_PLATFORM_WINDOWS
    #else
		#error "x86 Builds are not supported!"
    #endif
#endif

/*
#ifndef PROJECT_PATH
    #define PROJECT_PATH ""
#endif
*/
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