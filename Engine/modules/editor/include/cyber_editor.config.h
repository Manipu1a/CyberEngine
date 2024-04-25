#pragma once

#ifdef __cplusplus
    #define CYBER_EDITOR_EXTERN_C extern "C"
#else
    #define CYBERCYBER_EDITOR_EXTERN_C
#endif

#ifndef CYBER_EDITOR_EXPORT
    #if defined (_MSC_VER)
        #define CYBER_EDITOR_EXPORT __declspec(dllexport)
    #else
        #define CYBER_EDITOR_EXPORT
    #endif
#endif

#ifndef CYBER_EDITOR_API
    #define CYBER_EDITOR_API CYBER_EDITOR_EXPORT
#endif