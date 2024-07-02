#include "editor/imgui_impl_win32.h"
#include "imgui/backends/imgui_impl_win32.h"

namespace Cyber
{
    namespace Editor
    {
        Editor_Impl_Win32::Editor_Impl_Win32(const EditorCreateInfo& createInfo) : Editor(createInfo)
        {
            ImGui_ImplWin32_Init(createInfo.Hwnd);
        }

        Editor_Impl_Win32::~Editor_Impl_Win32()
        {
            ImGui_ImplWin32_Shutdown();
        }

        void Editor_Impl_Win32::new_frame(uint32_t renderSurfaceWidth, uint32_t renderSurfaceHeight)
        {
            ImGui_ImplWin32_NewFrame();
            Editor::new_frame(renderSurfaceWidth, renderSurfaceHeight);
        }
    }
}