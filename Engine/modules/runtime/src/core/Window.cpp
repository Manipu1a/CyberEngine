#include "core/Window.h"
#include "platform/WindowsWindow.h"

namespace Cyber
{
    Ref<Cyber::Window> Window::createWindow(const WindowDesc& desc)
    {
        return CreateRef<WindowsWindow>(desc);
    }
}