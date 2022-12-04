#pragma once

#ifdef __cplusplus
    #define CYBER_LOG_EXTERN_C extern "C"
#else
    #define CYBERCYBER_LOG_EXTERN_C
#endif

#ifndef CYBER_LOG_EXPORT
    #if defined (_MSC_VER)
        #define CYBER_LOG_EXPORT __declspec(dllexport)
    #else
        #define CYBER_LOG_EXPORT
    #endif
#endif

#ifndef CYBER_LOG_API
    #define CYBER_LOG_API CYBER_LOG_EXPORT
#endif