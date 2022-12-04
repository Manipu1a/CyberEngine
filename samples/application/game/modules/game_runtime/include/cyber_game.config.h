#pragma once

#ifndef CYBER_GAME_EXPORT
    #if defined (_MSC_VER)
        #define CYBER_GAME_EXPORT __declspec(dllexport)
    #else
        #define CYBER_GAME_EXPORT
    #endif
#endif

#ifndef CYBER_GAME_API
    #define CYBER_GAME_API CYBER_GAME_EXPORT
#endif