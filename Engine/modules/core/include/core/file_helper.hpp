#pragma once
#include "basic_file.h"
#include "platform/memory.h"
#include "core/config.h"
#include <filesystem>

CYBER_BEGIN_NAMESPACE(Cyber)
CYBER_BEGIN_NAMESPACE(Core)

class CYBER_CORE_API FileHelper
{
public:
    FileHelper(): file(nullptr) {}

    explicit FileHelper(const char* filePath, FILE_ACCESS_MODE accessMode = FILE_ACCESS_READ)
    {
        eastl::string full_path = resolve_path(filePath);
        FileOpenAttributes attributes(full_path.c_str(), accessMode);
        file = cyber_new<BasicFile>(attributes);
    }

    static void set_project_root(const char* root)
    {
        project_root = root;
    }

    static const char* get_project_root()
    {
        if (project_root.empty())
        {
            // Use PROJECT_PATH from config.h
            #ifdef PROJECT_PATH
                project_root = PROJECT_PATH;
            #else
                // Fallback to auto-detection if PROJECT_PATH is not defined
                project_root = auto_detect_project_root();
            #endif
        }
        return project_root.c_str();
    }

    operator BasicFile*() {return file;}
    BasicFile* operator->() {return file;}

private:
    // Fallback auto-detection function (only used if PROJECT_PATH is not defined)
    static eastl::string auto_detect_project_root()
    {
        std::filesystem::path cwd = std::filesystem::current_path();
        std::filesystem::path search_path = cwd;
        
        // Search upwards for project root markers
        while (!search_path.empty())
        {
            bool has_engine = std::filesystem::exists(search_path / "Engine");
            bool has_samples = std::filesystem::exists(search_path / "samples");
            bool has_cmake = std::filesystem::exists(search_path / "CMakeLists.txt");
            bool has_tools = std::filesystem::exists(search_path / "tools");
            
            int indicators = (has_engine ? 1 : 0) + (has_samples ? 1 : 0) + 
                            (has_cmake ? 1 : 0) + (has_tools ? 1 : 0);
            
            if (indicators >= 2)
            {
                return eastl::string(search_path.string().c_str());
            }
            
            auto parent = search_path.parent_path();
            if (parent == search_path) break;
            search_path = parent;
        }
        
        // Final fallback: use current directory
        return eastl::string(cwd.string().c_str());
    }

    static eastl::string resolve_path(const char* filePath)
    {
        // Check if absolute path
        std::filesystem::path path(filePath);
        if (path.is_absolute())
        {
            return eastl::string(filePath);
        }
        
        // Get project root (will auto-detect if needed)
        std::filesystem::path root_path(get_project_root());
        std::filesystem::path full_path = root_path / path;
        
        // Normalize the path to handle ../.. etc
        full_path = full_path.lexically_normal();
        
        // For existing files, verify the path
        if (std::filesystem::exists(full_path))
        {
            return eastl::string(full_path.string().c_str());
        }
        
        // Try relative to current directory as second option
        std::filesystem::path current_relative = std::filesystem::current_path() / path;
        current_relative = current_relative.lexically_normal();
        
        if (std::filesystem::exists(current_relative))
        {
            return eastl::string(current_relative.string().c_str());
        }
        
        // Return the project-root-relative path for new files
        return eastl::string(full_path.string().c_str());
    }

private:
    BasicFile* file;
    static eastl::string project_root;
};

// Static member definition
inline eastl::string FileHelper::project_root;

CYBER_END_NAMESPACE
CYBER_END_NAMESPACE