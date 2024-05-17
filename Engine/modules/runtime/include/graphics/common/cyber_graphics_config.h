#pragma once
#include "platform/configure.h"
#include "tools/hash.h"

#ifndef CYBER_GRAPHICS_EXPORT
    #if defined (_MSC_VER)
        #define CYBER_GRAPHICS_EXPORT __declspec(dllexport)
    #else
        #define CYBER_GRAPHICS_EXPORT
    #endif
#endif

#ifndef CYBER_GRAPHICS_IMPORT
    #if defined (_MSC_VER)
        #define CYBER_GRAPHICS_IMPORT __declspec(dllimport)
    #else
        #define CYBER_GRAPHICS_IMPORT
    #endif
#endif

#ifndef CYBER_GRAPHICS_API
    #if defined (CYBER_API_EXPORT)
        #define CYBER_GRAPHICS_API CYBER_GRAPHICS_EXPORT
    #else
        #define CYBER_GRAPHICS_API CYBER_GRAPHICS_IMPORT
    #endif
#endif

#if defined (_WINDOWS)
    #include "graphics/backend/d3d12/d3d12.config.h"
#else

#endif

#if UINTPTR_MAX == UINT32_MAX
    #define GRAPHICS_NAME_HASH_SEED 1610612741
#else
    #define GRAPHICS_NAME_HASH_SEED 8053064571610612741
#endif

#ifndef graphics_max
    #define graphics_max(a, b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef graphics_min
    #define graphics_min(a, b) (((a) < (b)) ? (a) : (b))
#endif
#define PSO_NAME_LENGTH 160

#define graphics_hash(buffer, size, seed) cyber_hash(buffer, size, seed)
#define graphics_name_hash(buffer, size) cyber_hash(buffer, size, GRAPHICS_NAME_HASH_SEED)