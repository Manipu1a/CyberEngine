#include "gameruntime/project_settings.h"

#include "core/file_helper.hpp"
#include "log/Log.h"

#include <nlohmann/json.hpp>
#include <fstream>

namespace Cyber
{
    namespace ProjectSettingsIO
    {
        namespace
        {
            using json = nlohmann::json;
            namespace fs = std::filesystem;
        }

        std::filesystem::path settings_path()
        {
            fs::path project_root = fs::path(Core::FileHelper::get_project_root()).lexically_normal();
            if (project_root.empty())
                return {};
            return project_root / "ProjectSettings.json";
        }

        eastl::string make_project_relative_path(const std::filesystem::path& path)
        {
            if (path.empty())
                return {};

            fs::path normalized = path.lexically_normal();
            if (!normalized.is_absolute())
                return eastl::string(normalized.generic_string().c_str());

            std::error_code ec;
            fs::path project_root = fs::path(Core::FileHelper::get_project_root()).lexically_normal();
            fs::path relative = fs::relative(normalized, project_root, ec);
            if (!ec && !relative.empty() && relative.native().front() != '.')
                return eastl::string(relative.generic_string().c_str());
            return eastl::string(normalized.string().c_str());
        }

        bool load(ProjectSettings& out_settings)
        {
            out_settings = {};

            const fs::path path = settings_path();
            if (path.empty())
                return false;

            std::error_code ec;
            if (!fs::exists(path, ec))
                return true;

            std::ifstream ifs(path);
            if (!ifs.is_open())
            {
                CB_WARN("ProjectSettings: failed to open {}", path.string().c_str());
                return false;
            }

            json root;
            try
            {
                ifs >> root;
            }
            catch (const std::exception& e)
            {
                CB_WARN("ProjectSettings: json parse error in {} ({})", path.string().c_str(), e.what());
                return false;
            }

            if (!root.is_object())
                return false;

            if (root.contains("startupScene") && root["startupScene"].is_string())
                out_settings.startup_scene = root["startupScene"].get<std::string>().c_str();
            return true;
        }

        bool save(const ProjectSettings& settings)
        {
            const fs::path path = settings_path();
            if (path.empty())
                return false;

            std::error_code ec;
            if (path.has_parent_path())
                fs::create_directories(path.parent_path(), ec);
            if (ec)
            {
                CB_WARN("ProjectSettings: failed to create directory {} ({})",
                        path.parent_path().string().c_str(),
                        ec.message().c_str());
                return false;
            }

            json root;
            root["version"] = 1;
            root["startupScene"] = std::string(settings.startup_scene.c_str());

            std::ofstream ofs(path);
            if (!ofs.is_open())
            {
                CB_WARN("ProjectSettings: failed to write {}", path.string().c_str());
                return false;
            }

            ofs << root.dump(2);
            return true;
        }
    }
}
