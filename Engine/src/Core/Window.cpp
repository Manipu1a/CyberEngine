#include "Core/Window.h"
#include "Platform/WindowsWindow.h"

namespace Cyber
{
    Scope<Cyber::Window> Window::createWindow(const WindowDesc& desc)
    {
        return CreateScope<WindowsWindow>(desc);
    }
}