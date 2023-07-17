#pragma once

#ifndef CYBER_INPUTSYSTEM_EXPORT
    #if defined (_MSC_VER)
        #define CYBER_INPUTSYSTEM_EXPORT __declspec(dllexport)
    #else
        #define CYBER_INPUTSYSTEM_EXPORT
    #endif
#endif

#ifndef CYBER_INPUTSYSTEM_API
    #define CYBER_INPUTSYSTEM_API CYBER_INPUTSYSTEM_EXPORT
#endif