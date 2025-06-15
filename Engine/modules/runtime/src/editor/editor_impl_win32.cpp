#include "editor/editor_impl_win32.h"
#include "imgui/backends/imgui_impl_win32.h"
#include "graphics/interface/render_device.hpp"

namespace Cyber
{
    namespace Editor
    {
        Editor_Impl_Win32::Editor_Impl_Win32(const EditorCreateInfo& createInfo) : Editor(createInfo)
        {
        }

        Editor_Impl_Win32::~Editor_Impl_Win32()
        {
            ImGui_ImplWin32_Shutdown();
        }

        void Editor_Impl_Win32::initialize(RenderObject::IRenderDevice* device, HWND hwnd)
        {
            Editor::initialize(device, hwnd);

            ImGui_ImplWin32_Init(hwnd);
        }

        void Editor_Impl_Win32::new_frame(uint32_t renderSurfaceWidth, uint32_t renderSurfaceHeight)
        {
            ImGui_ImplWin32_NewFrame();
            Editor::new_frame(renderSurfaceWidth, renderSurfaceHeight);
        }

        LRESULT Editor_Impl_Win32::proc_msg_win32(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
        {
            if(ImGui::GetCurrentContext() == nullptr)
            {
                return 0;
            }

            //auto res = ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam);

            return 0;
        }

    }
}