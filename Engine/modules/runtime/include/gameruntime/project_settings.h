#pragma once

#include "cyber_runtime.config.h"
#include "EASTL/string.h"

#include <filesystem>

namespace Cyber
{
    struct CYBER_RUNTIME_API ProjectSettings
    {
        eastl::string startup_scene;
    };

    namespace ProjectSettingsIO
    {
        CYBER_RUNTIME_API std::filesystem::path settings_path();
        CYBER_RUNTIME_API eastl::string make_project_relative_path(const std::filesystem::path& path);
        CYBER_RUNTIME_API bool load(ProjectSettings& out_settings);
        CYBER_RUNTIME_API bool save(const ProjectSettings& settings);
    }
}
