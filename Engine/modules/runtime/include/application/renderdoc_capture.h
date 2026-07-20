#pragma once

#include "cyber_runtime.config.h"

namespace Cyber::Core::RenderDocCapture
{
    CYBER_RUNTIME_API bool initialize();
    CYBER_RUNTIME_API bool is_available();
    CYBER_RUNTIME_API bool connect_replay_ui();
    CYBER_RUNTIME_API bool begin_frame_capture(void* window_handle);
    CYBER_RUNTIME_API void end_frame_capture();
    CYBER_RUNTIME_API const char* get_capture_path_template();
}
