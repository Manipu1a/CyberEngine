#pragma once

#ifdef __cplusplus
    #define CYBER_GAME_EXTERN_C extern "C"
#else
    #define CYBERCYBER_GAME_EXTERN_C
#endif

#ifndef CYBER_GAME_EXPORT
    #if defined (_MSC_VER)
        #define CYBER_GAME_EXPORT __declspec(dllexport)
    #else
        #define CYBER_GAME_EXPORT
    #endif
#endif

#ifndef CYBER_GAME_IMPORT
    #if defined (_MSC_VER)
        #define CYBER_GAME_IMPORT __declspec(dllimport)
    #else
        #define CYBER_GAME_IMPORT
    #endif
#endif

#if defined (CYBER_API_EXPORT)
    #define CYBER_GAME_API CYBER_GAME_EXPORT
#else
    #define CYBER_GAME_API CYBER_GAME_IMPORT
#endif