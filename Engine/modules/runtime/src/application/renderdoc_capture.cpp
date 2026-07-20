#include "application/renderdoc_capture.h"

#include "core/file_helper.hpp"
#include "log/Log.h"

#include <Windows.h>
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace
{
    enum RENDERDOC_Version
    {
        eRENDERDOC_API_Version_1_0_0 = 10000,
    };

    using RENDERDOC_DevicePointer = void*;
    using RENDERDOC_WindowHandle = void*;
    using RENDERDOC_InputButton = int;

    using pRENDERDOC_GetAPIVersion = void(__cdecl*)(int* major, int* minor, int* patch);
    using pRENDERDOC_SetCaptureOptionU32 = int(__cdecl*)(int option, uint32_t value);
    using pRENDERDOC_SetCaptureOptionF32 = int(__cdecl*)(int option, float value);
    using pRENDERDOC_GetCaptureOptionU32 = uint32_t(__cdecl*)(int option);
    using pRENDERDOC_GetCaptureOptionF32 = float(__cdecl*)(int option);
    using pRENDERDOC_SetFocusToggleKeys = void(__cdecl*)(RENDERDOC_InputButton* keys, int num);
    using pRENDERDOC_SetCaptureKeys = void(__cdecl*)(RENDERDOC_InputButton* keys, int num);
    using pRENDERDOC_GetOverlayBits = uint32_t(__cdecl*)();
    using pRENDERDOC_MaskOverlayBits = void(__cdecl*)(uint32_t and_mask, uint32_t or_mask);
    using pRENDERDOC_RemoveHooks = void(__cdecl*)();
    using pRENDERDOC_UnloadCrashHandler = void(__cdecl*)();
    using pRENDERDOC_SetCaptureFilePathTemplate = void(__cdecl*)(const char* path_template);
    using pRENDERDOC_GetCaptureFilePathTemplate = const char*(__cdecl*)();
    using pRENDERDOC_GetNumCaptures = uint32_t(__cdecl*)();
    using pRENDERDOC_GetCapture = uint32_t(__cdecl*)(uint32_t index, char* filename, uint32_t* path_length, uint64_t* timestamp);
    using pRENDERDOC_TriggerCapture = void(__cdecl*)();
    using pRENDERDOC_IsTargetControlConnected = uint32_t(__cdecl*)();
    using pRENDERDOC_LaunchReplayUI = uint32_t(__cdecl*)(uint32_t connect_target_control, const char* cmdline);
    using pRENDERDOC_SetActiveWindow = void(__cdecl*)(RENDERDOC_DevicePointer device, RENDERDOC_WindowHandle window);
    using pRENDERDOC_StartFrameCapture = void(__cdecl*)(RENDERDOC_DevicePointer device, RENDERDOC_WindowHandle window);
    using pRENDERDOC_IsFrameCapturing = uint32_t(__cdecl*)();
    using pRENDERDOC_EndFrameCapture = uint32_t(__cdecl*)(RENDERDOC_DevicePointer device, RENDERDOC_WindowHandle window);

    struct RENDERDOC_API_1_0_0
    {
        pRENDERDOC_GetAPIVersion GetAPIVersion;
        pRENDERDOC_SetCaptureOptionU32 SetCaptureOptionU32;
        pRENDERDOC_SetCaptureOptionF32 SetCaptureOptionF32;
        pRENDERDOC_GetCaptureOptionU32 GetCaptureOptionU32;
        pRENDERDOC_GetCaptureOptionF32 GetCaptureOptionF32;
        pRENDERDOC_SetFocusToggleKeys SetFocusToggleKeys;
        pRENDERDOC_SetCaptureKeys SetCaptureKeys;
        pRENDERDOC_GetOverlayBits GetOverlayBits;
        pRENDERDOC_MaskOverlayBits MaskOverlayBits;
        pRENDERDOC_RemoveHooks RemoveHooks;
        pRENDERDOC_UnloadCrashHandler UnloadCrashHandler;
        pRENDERDOC_SetCaptureFilePathTemplate SetCaptureFilePathTemplate;
        pRENDERDOC_GetCaptureFilePathTemplate GetCaptureFilePathTemplate;
        pRENDERDOC_GetNumCaptures GetNumCaptures;
        pRENDERDOC_GetCapture GetCapture;
        pRENDERDOC_TriggerCapture TriggerCapture;
        pRENDERDOC_IsTargetControlConnected IsTargetControlConnected;
        pRENDERDOC_LaunchReplayUI LaunchReplayUI;
        pRENDERDOC_SetActiveWindow SetActiveWindow;
        pRENDERDOC_StartFrameCapture StartFrameCapture;
        pRENDERDOC_IsFrameCapturing IsFrameCapturing;
        pRENDERDOC_EndFrameCapture EndFrameCapture;
    };

    using pRENDERDOC_GetAPI = int(__cdecl*)(RENDERDOC_Version version, void** out_api_pointers);

    HMODULE g_renderdoc_module = nullptr;
    RENDERDOC_API_1_0_0* g_renderdoc_api = nullptr;
    bool g_initialization_attempted = false;
    bool g_capture_active = false;
    void* g_capture_window = nullptr;
    std::string g_capture_path_template;

    bool env_flag_is_disabled(const wchar_t* name)
    {
        wchar_t value[16] = {};
        DWORD len = GetEnvironmentVariableW(name, value, static_cast<DWORD>(std::size(value)));
        return len > 0 && len < std::size(value) && (value[0] == L'0' || value[0] == L'n' || value[0] == L'N');
    }

    std::wstring get_env_path(const wchar_t* name)
    {
        wchar_t value[32768] = {};
        DWORD len = GetEnvironmentVariableW(name, value, static_cast<DWORD>(std::size(value)));
        if (len == 0 || len >= std::size(value))
            return {};

        return std::wstring(value, len);
    }

    std::wstring get_env_dir(const wchar_t* name)
    {
        std::wstring value = get_env_path(name);
        if (!value.empty() && value.back() != L'\\' && value.back() != L'/')
            value.push_back(L'\\');
        return value;
    }

    HMODULE load_renderdoc_from_path(const std::filesystem::path& path)
    {
        std::error_code ec;
        if (path.empty() || !std::filesystem::exists(path, ec))
            return nullptr;

        HMODULE module = LoadLibraryExW(
            path.c_str(),
            nullptr,
            LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);

        if (!module)
            module = LoadLibraryW(path.c_str());

        if (module)
            CB_CORE_INFO("RenderDoc loaded from {0}", path.string());

        return module;
    }

    std::filesystem::path get_project_root_path()
    {
        std::filesystem::path project_root(Cyber::Core::FileHelper::get_project_root());
        if (project_root.empty())
            project_root = std::filesystem::current_path();

        return project_root.lexically_normal();
    }

    HMODULE load_renderdoc_module()
    {
        HMODULE module = GetModuleHandleW(L"renderdoc.dll");
        if (module)
        {
            CB_CORE_INFO("RenderDoc module already loaded.");
            return module;
        }

        std::wstring configured_path = get_env_path(L"CYBER_RENDERDOC_DLL");
        if (!configured_path.empty())
        {
            module = load_renderdoc_from_path(configured_path);
            if (!module)
                CB_CORE_WARN("CYBER_RENDERDOC_DLL is set, but RenderDoc could not be loaded from it.");
            return module;
        }

        std::vector<std::filesystem::path> candidates;
        std::filesystem::path project_root = get_project_root_path();
        std::wstring program_files = get_env_dir(L"ProgramFiles");
        std::wstring program_files_x86 = get_env_dir(L"ProgramFiles(x86)");

        candidates.emplace_back(project_root / "Engine" / "Plugins" / "RenderDoc" / "bin" / "renderdoc.dll");
        candidates.emplace_back(project_root / "plugins" / "renderdoc" / "bin" / "renderdoc.dll");

        if (!program_files.empty())
            candidates.emplace_back(program_files + L"RenderDoc\\renderdoc.dll");
        if (!program_files_x86.empty())
            candidates.emplace_back(program_files_x86 + L"RenderDoc\\renderdoc.dll");

        for (const auto& candidate : candidates)
        {
            module = load_renderdoc_from_path(candidate);
            if (module)
                return module;
        }

        return nullptr;
    }

    std::string path_to_utf8_template(const std::filesystem::path& path)
    {
        const std::u8string utf8 = path.generic_u8string();
        return std::string(reinterpret_cast<const char*>(utf8.data()), utf8.size());
    }

    void configure_capture_path()
    {
        std::filesystem::path capture_dir = get_project_root_path() / "captures";

        std::error_code ec;
        std::filesystem::create_directories(capture_dir, ec);
        if (ec)
        {
            CB_CORE_WARN("RenderDoc capture directory could not be created: {0}", ec.message());
            capture_dir = std::filesystem::current_path();
        }

        g_capture_path_template = path_to_utf8_template((capture_dir / "CyberEngine").lexically_normal());

        if (g_renderdoc_api && g_renderdoc_api->SetCaptureFilePathTemplate)
            g_renderdoc_api->SetCaptureFilePathTemplate(g_capture_path_template.c_str());
    }

    std::string get_latest_capture_path()
    {
        if (!g_renderdoc_api || !g_renderdoc_api->GetNumCaptures || !g_renderdoc_api->GetCapture)
            return {};

        uint32_t capture_count = g_renderdoc_api->GetNumCaptures();
        if (capture_count == 0)
            return {};

        uint32_t path_length = 0;
        if (!g_renderdoc_api->GetCapture(capture_count - 1, nullptr, &path_length, nullptr) || path_length == 0)
            return {};

        std::string path(path_length + 1, '\0');
        if (!g_renderdoc_api->GetCapture(capture_count - 1, path.data(), &path_length, nullptr))
            return {};

        path.resize(path_length);
        return path;
    }

    bool launch_connected_replay_ui()
    {
        if (!g_renderdoc_api || !g_renderdoc_api->LaunchReplayUI)
            return false;

        if (g_renderdoc_api->IsTargetControlConnected && g_renderdoc_api->IsTargetControlConnected())
        {
            CB_CORE_INFO("RenderDoc target control is already connected.");
            return true;
        }

        uint32_t pid = g_renderdoc_api->LaunchReplayUI(1, nullptr);
        if (pid == 0)
        {
            CB_CORE_WARN("RenderDoc replay UI failed to launch for target control attach.");
            return false;
        }

        CB_CORE_INFO("RenderDoc replay UI launched for target control attach. PID: {0}", pid);
        return true;
    }
}

namespace Cyber::Core::RenderDocCapture
{
    bool initialize()
    {
        if (g_initialization_attempted)
            return g_renderdoc_api != nullptr;

        g_initialization_attempted = true;

        if (env_flag_is_disabled(L"CYBER_RENDERDOC_ENABLE"))
        {
            CB_CORE_INFO("RenderDoc hook loading disabled by CYBER_RENDERDOC_ENABLE=0.");
            return false;
        }

        g_renderdoc_module = load_renderdoc_module();
        if (!g_renderdoc_module)
        {
            CB_CORE_WARN("RenderDoc DLL not found. Set CYBER_RENDERDOC_DLL to enable startup hook loading.");
            return false;
        }

        auto get_api = reinterpret_cast<pRENDERDOC_GetAPI>(GetProcAddress(g_renderdoc_module, "RENDERDOC_GetAPI"));
        if (!get_api)
        {
            CB_CORE_WARN("RenderDoc DLL does not export RENDERDOC_GetAPI.");
            return false;
        }

        void* api = nullptr;
        if (get_api(eRENDERDOC_API_Version_1_0_0, &api) != 1 || !api)
        {
            CB_CORE_WARN("RenderDoc API 1.0.0 is not available.");
            return false;
        }

        g_renderdoc_api = static_cast<RENDERDOC_API_1_0_0*>(api);

        if (g_renderdoc_api->SetCaptureKeys)
            g_renderdoc_api->SetCaptureKeys(nullptr, 0);
        if (g_renderdoc_api->SetFocusToggleKeys)
            g_renderdoc_api->SetFocusToggleKeys(nullptr, 0);
        if (g_renderdoc_api->MaskOverlayBits)
            g_renderdoc_api->MaskOverlayBits(0, 0);

        configure_capture_path();

        int major = 0;
        int minor = 0;
        int patch = 0;
        if (g_renderdoc_api->GetAPIVersion)
            g_renderdoc_api->GetAPIVersion(&major, &minor, &patch);

        CB_CORE_INFO("RenderDoc hook initialized ({0}.{1}.{2}). Press F3 to attach RenderDoc UI. Capture path template: {3}",
            major,
            minor,
            patch,
            g_capture_path_template);

        return true;
    }

    bool is_available()
    {
        return g_renderdoc_api != nullptr;
    }

    bool connect_replay_ui()
    {
        return launch_connected_replay_ui();
    }

    bool begin_frame_capture(void* window_handle)
    {
        if (!g_renderdoc_api)
            return false;

        if (g_capture_active)
            return false;

        if (g_renderdoc_api->IsFrameCapturing && g_renderdoc_api->IsFrameCapturing())
        {
            CB_CORE_WARN("RenderDoc is already capturing a frame.");
            return false;
        }

        g_capture_window = window_handle;

        if (g_renderdoc_api->StartFrameCapture)
            g_renderdoc_api->StartFrameCapture(nullptr, g_capture_window);

        g_capture_active = true;

        CB_CORE_INFO("RenderDoc frame capture started.");
        return true;
    }

    void end_frame_capture()
    {
        if (!g_capture_active || !g_renderdoc_api)
            return;

        uint32_t result = 0;
        if (g_renderdoc_api->EndFrameCapture)
            result = g_renderdoc_api->EndFrameCapture(nullptr, g_capture_window);

        g_capture_active = false;
        g_capture_window = nullptr;

        if (result)
        {
            std::string capture_path = get_latest_capture_path();
            if (!capture_path.empty())
            {
                CB_CORE_INFO("RenderDoc frame capture saved: {0}", capture_path);
            }
            else
            {
                CB_CORE_INFO("RenderDoc frame capture saved.");
            }
        }
        else
        {
            CB_CORE_WARN("RenderDoc frame capture did not save a capture.");
        }
    }

    const char* get_capture_path_template()
    {
        return g_capture_path_template.c_str();
    }
}
