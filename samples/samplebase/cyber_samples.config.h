#pragma once

#ifdef __cplusplus
    #define CYBER_SAMPLES_EXTERN_C extern "C"
#else
    #define CYBERCYBER_SAMPLES_EXTERN_C
#endif

#ifndef CYBER_SAMPLES_EXPORT
    #if defined (_MSC_VER)
        #define CYBER_SAMPLES_EXPORT __declspec(dllexport)
    #else
        #define CYBER_SAMPLES_EXPORT
    #endif
#endif

#ifndef CYBER_SAMPLES_IMPORT
    #if defined (_MSC_VER)
        #define CYBER_SAMPLES_IMPORT __declspec(dllimport)
    #else
        #define CYBER_SAMPLES_IMPORT
    #endif
#endif

#if defined (CYBER_API_EXPORT)
    #define CYBER_SAMPLES_API CYBER_SAMPLES_EXPORT
#else
    #define CYBER_SAMPLES_API CYBER_SAMPLES_IMPORT
#endif