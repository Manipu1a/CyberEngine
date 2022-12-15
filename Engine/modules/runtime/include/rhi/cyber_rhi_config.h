#pragma once

#ifndef CYBER_RHI_EXPORT
    #if defined (_MSC_VER)
        #define CYBER_RHI_EXPORT __declspec(dllexport)
    #else
        #define CYBER_RHI_EXPORT
    #endif
#endif

#ifndef CYBER_RHI_API
    #define CYBER_RHI_API CYBER_RHI_EXPORT
#endif

#if defined (_WINDOWS)
    #include "backend/d3d12/d3d12.config.h"
#else

#endif