

--target("Engine")
--set_kind("binary")
--set_languages("cxx17")

add_files(source_list)
add_includedirs(include_dir_list, {public = true})
set_pcheader("$(projectdir)/Engine/cepch.h")
includes("Modules/xmake.lua")

-- Convert backslashes to forward slashes for cross-platform compatibility
-- Method 1: Use forward slashes (recommended for cross-platform)
local project_path = os.projectdir():gsub("\\", "/")
-- Method 2: Use double backslashes (Windows specific)
-- local project_path = os.projectdir():gsub("\\", "\\\\")
set_configvar("PROJECT_PATH", project_path)
-- set_configvar("PROJECT_PATH", "$(projectdir)")
