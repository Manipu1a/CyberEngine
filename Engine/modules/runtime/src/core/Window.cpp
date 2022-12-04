#include "core/Window.h"
#include "platform/WindowsWindow.h"

namespace Cyber
{
    Scope<Cyber::Window> Window::createWindow(const WindowDesc& desc)
    {
        return CreateScope<WindowsWindow>(desc);
    }
}