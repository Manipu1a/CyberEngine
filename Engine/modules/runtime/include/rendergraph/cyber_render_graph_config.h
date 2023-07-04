#pragma once
#include "../platform/configure.h"

#ifndef CYBER_RENDER_GRAPH_EXPORT
    #if defined (_MSC_VER)
        #define CYBER_RENDER_GRAPH_EXPORT __declspec(dllexport)
    #else
        #define CYBER_RENDER_GRAPH_EXPORT
    #endif
#endif

#ifndef CYBER_RENDER_GRAPH_API
    #define CYBER_RENDER_GRAPH_API CYBER_RENDER_GRAPH_EXPORT
#endif
