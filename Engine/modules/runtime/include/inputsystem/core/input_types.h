#pragma once

#include <cstdint>
#include <functional>
#include <chrono>

namespace CyberEngine::Input {

using DeviceID = uint32_t;
using ButtonID = uint32_t;
using ActionID = uint32_t;
using AxisID = uint32_t;

constexpr DeviceID INVALID_DEVICE_ID = static_cast<DeviceID>(-1);
constexpr ButtonID INVALID_BUTTON_ID = static_cast<ButtonID>(-1);
constexpr ActionID INVALID_ACTION_ID = static_cast<ActionID>(-1);

enum class DeviceType : uint8_t {
    Unknown = 0,
    Keyboard,
    Mouse,
    Gamepad,
    Touch,
    Custom
};

enum class ButtonState : uint8_t {
    Released = 0,
    Pressed = 1,
    Held = 2
};

enum class InputEventType : uint8_t {
    ButtonDown,
    ButtonUp,
    AxisMove,
    DeviceConnected,
    DeviceDisconnected
};

struct InputEvent {
    InputEventType type;
    DeviceID deviceId;
    DeviceType deviceType;
    union {
        struct {
            ButtonID buttonId;
            ButtonState state;
        } button;
        struct {
            AxisID axisId;
            float value;
            float delta;
        } axis;
    };
    std::chrono::steady_clock::time_point timestamp;
};

enum class Key : uint16_t {
    Unknown = 0,
    
    // Letters
    A, B, C, D, E, F, G, H, I, J, K, L, M,
    N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
    
    // Numbers
    Num0, Num1, Num2, Num3, Num4, 
    Num5, Num6, Num7, Num8, Num9,
    
    // Function keys
    F1, F2, F3, F4, F5, F6, F7, F8, 
    F9, F10, F11, F12, F13, F14, F15,
    
    // Control keys
    Escape, Tab, CapsLock, Shift, Control, Alt,
    Space, Enter, Backspace, Delete,
    
    // Arrow keys
    Up, Down, Left, Right,
    
    // Special keys
    Insert, Home, End, PageUp, PageDown,
    PrintScreen, ScrollLock, Pause,
    
    // Numpad
    Numpad0, Numpad1, Numpad2, Numpad3, Numpad4,
    Numpad5, Numpad6, Numpad7, Numpad8, Numpad9,
    NumpadAdd, NumpadSubtract, NumpadMultiply, NumpadDivide,
    NumpadEnter, NumpadDecimal, NumpadLock,
    
    Count
};

enum class MouseButton : uint8_t {
    Left = 0,
    Right,
    Middle,
    X1,
    X2,
    Count
};

enum class MouseAxis : uint8_t {
    X = 0,
    Y,
    Wheel,
    HorizontalWheel,
    Count
};

enum class GamepadButton : uint16_t {
    A = 0,
    B,
    X,
    Y,
    LeftBumper,
    RightBumper,
    Back,
    Start,
    Guide,
    LeftThumb,
    RightThumb,
    DPadUp,
    DPadRight,
    DPadDown,
    DPadLeft,
    LeftTrigger,
    RightTrigger,
    Count
};

enum class GamepadAxis : uint8_t {
    LeftX = 0,
    LeftY,
    RightX,
    RightY,
    LeftTrigger,
    RightTrigger,
    Count
};

struct InputBinding {
    DeviceType deviceType;
    DeviceID deviceId;
    ButtonID buttonId;
    float scale = 1.0f;
    float deadZone = 0.0f;
};

using InputCallback = std::function<void(const InputEvent&)>;
using ActionCallback = std::function<void(float value)>;

} // namespace CyberEngine::Input