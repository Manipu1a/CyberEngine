#include "inputsystem/platform/win32/Input_backend_win32.h"
#include <Windows.h>
#include <windowsx.h>

namespace Cyber::Input {

void InputBackendWin32::process_platform_event(void* nativeEvent) {
    if (!nativeEvent) return;
    
    MSG* msg = static_cast<MSG*>(nativeEvent);
    
    // Get keyboard device
    KeyboardDevice* keyboard = dynamic_cast<KeyboardDevice*>(m_keyboard);
    MouseDevice* mouse = dynamic_cast<MouseDevice*>(m_mouse);
    
    switch (msg->message) {
        // Keyboard events
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
            if (keyboard) {
                Key key = KeyboardDevice::virtual_key_to_key(static_cast<int>(msg->wParam));
                if (key != Key::Unknown) {
                    keyboard->handle_key_down(key);
                }
            }
            break;
            
        case WM_KEYUP:
        case WM_SYSKEYUP:
            if (keyboard) {
                Key key = KeyboardDevice::virtual_key_to_key(static_cast<int>(msg->wParam));
                if (key != Key::Unknown) {
                    keyboard->handle_key_up(key);
                }
            }
            break;
            
        case WM_CHAR:
            if (keyboard) {
                keyboard->handle_char(static_cast<wchar_t>(msg->wParam));
            }
            break;
            
        // Mouse events
        case WM_LBUTTONDOWN:
            if (mouse) {
                mouse->handle_button_down(MouseButton::Left);
            }
            break;
            
        case WM_LBUTTONUP:
            if (mouse) {
                mouse->handle_button_up(MouseButton::Left);
            }
            break;
            
        case WM_RBUTTONDOWN:
            if (mouse) {
                mouse->handle_button_down(MouseButton::Right);
            }
            break;
            
        case WM_RBUTTONUP:
            if (mouse) {
                mouse->handle_button_up(MouseButton::Right);
            }
            break;
            
        case WM_MBUTTONDOWN:
            if (mouse) {
                mouse->handle_button_down(MouseButton::Middle);
            }
            break;
            
        case WM_MBUTTONUP:
            if (mouse) {
                mouse->handle_button_up(MouseButton::Middle);
            }
            break;
            
        case WM_XBUTTONDOWN:
            if (mouse) {
                int button = GET_XBUTTON_WPARAM(msg->wParam);
                if (button == XBUTTON1) {
                    mouse->handle_button_down(MouseButton::X1);
                } else if (button == XBUTTON2) {
                    mouse->handle_button_down(MouseButton::X2);
                }
            }
            break;
            
        case WM_XBUTTONUP:
            if (mouse) {
                int button = GET_XBUTTON_WPARAM(msg->wParam);
                if (button == XBUTTON1) {
                    mouse->handle_button_up(MouseButton::X1);
                } else if (button == XBUTTON2) {
                    mouse->handle_button_up(MouseButton::X2);
                }
            }
            break;
            
        case WM_MOUSEMOVE:
            if (mouse) {
                float x = static_cast<float>(GET_X_LPARAM(msg->lParam));
                float y = static_cast<float>(GET_Y_LPARAM(msg->lParam));
                mouse->handle_move(x, y);
            }
            break;
            
        case WM_MOUSEWHEEL:
            if (mouse) {
                float delta = static_cast<float>(GET_WHEEL_DELTA_WPARAM(msg->wParam)) / WHEEL_DELTA;
                mouse->handle_wheel(delta);
            }
            break;
            
        case WM_MOUSEHWHEEL:
            if (mouse) {
                float delta = static_cast<float>(GET_WHEEL_DELTA_WPARAM(msg->wParam)) / WHEEL_DELTA;
                mouse->handle_horizontal_wheel(delta);
            }
            break;
    }
}

} // namespace CyberEngine::Input