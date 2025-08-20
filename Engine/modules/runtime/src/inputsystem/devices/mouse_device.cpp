#include "inputsystem/devices/mouse_device.h"
#include <chrono>

#ifdef _WIN32
#include <Windows.h>
#endif

namespace Cyber::Input {

MouseDevice::MouseDevice(DeviceID id)
    : IInputDevice(id, DeviceType::Mouse) {
}

bool MouseDevice::initialize() {
    m_connected = true;
    return true;
}

void MouseDevice::shutdown() {
    m_connected = false;
}

void MouseDevice::update(float deltaTime) {
    // Update just pressed/released states
    for (size_t i = 0; i < m_buttonStates.size(); ++i) {
        m_buttonStates[i].justPressed = m_buttonStates[i].pressed && !m_previousButtonStates[i].pressed;
        m_buttonStates[i].justReleased = !m_buttonStates[i].pressed && m_previousButtonStates[i].pressed;
    }
    
    m_previousButtonStates = m_buttonStates;
}

void MouseDevice::end_frame()
{
    // Reset deltas at the end of frame
    reset_deltas();
}

ButtonState MouseDevice::get_button_state(ButtonID button) const {
    if (button < static_cast<ButtonID>(MouseButton::Count)) {
        return m_buttonStates[button].pressed ? ButtonState::Pressed : ButtonState::Released;
    }
    return ButtonState::Released;
}

float MouseDevice::get_axis_value(AxisID axis) const {
    switch (static_cast<MouseAxis>(axis)) {
        case MouseAxis::X:
            return m_positionX;
        case MouseAxis::Y:
            return m_positionY;
        case MouseAxis::Wheel:
            return m_wheelDelta;
        case MouseAxis::HorizontalWheel:
            return m_horizontalWheelDelta;
        default:
            return 0.0f;
    }
}

float MouseDevice::get_axis_delta(AxisID axis) const {
    switch (static_cast<MouseAxis>(axis)) {
        case MouseAxis::X:
            return m_deltaX;
        case MouseAxis::Y:
            return m_deltaY;
        case MouseAxis::Wheel:
            return m_wheelDelta;
        case MouseAxis::HorizontalWheel:
            return m_horizontalWheelDelta;
        default:
            return 0.0f;
    }
}

bool MouseDevice::is_button_pressed(ButtonID button) const {
    if (button < static_cast<ButtonID>(MouseButton::Count)) {
        return m_buttonStates[button].pressed;
    }
    return false;
}

bool MouseDevice::is_button_just_pressed(ButtonID button) const {
    if (button < static_cast<ButtonID>(MouseButton::Count)) {
        return m_buttonStates[button].justPressed;
    }
    return false;
}

bool MouseDevice::is_button_just_released(ButtonID button) const {
    if (button < static_cast<ButtonID>(MouseButton::Count)) {
        return m_buttonStates[button].justReleased;
    }
    return false;
}

void MouseDevice::handle_button_down(MouseButton button) {
    if (button < MouseButton::Count) {
        update_button_state(button, true);
        
        InputEvent event;
        event.type = InputEventType::ButtonDown;
        event.deviceId = m_deviceId;
        event.deviceType = m_deviceType;
        event.button.buttonId = static_cast<ButtonID>(button);
        event.button.state = ButtonState::Pressed;
        event.timestamp = std::chrono::steady_clock::now();
        add_event(event);
    }
}

void MouseDevice::handle_button_up(MouseButton button) {
    if (button < MouseButton::Count) {
        update_button_state(button, false);
        
        InputEvent event;
        event.type = InputEventType::ButtonUp;
        event.deviceId = m_deviceId;
        event.deviceType = m_deviceType;
        event.button.buttonId = static_cast<ButtonID>(button);
        event.button.state = ButtonState::Released;
        event.timestamp = std::chrono::steady_clock::now();
        add_event(event);
    }
}

void MouseDevice::handle_move(float x, float y) {
    if (m_firstMove) {
        m_previousX = x;
        m_previousY = y;
        m_firstMove = false;
    }
    
    m_deltaX = x - m_previousX;
    m_deltaY = y - m_previousY;
    
    m_positionX = x;
    m_positionY = y;
    
    m_previousX = x;
    m_previousY = y;
    
    // Generate axis move events
    if (m_deltaX != 0.0f) {
        InputEvent event;
        event.type = InputEventType::AxisMove;
        event.deviceId = m_deviceId;
        event.deviceType = m_deviceType;
        event.axis.axisId = static_cast<AxisID>(MouseAxis::X);
        event.axis.value = m_positionX;
        event.axis.delta = m_deltaX;
        event.timestamp = std::chrono::steady_clock::now();
        add_event(event);
    }
    
    if (m_deltaY != 0.0f) {
        InputEvent event;
        event.type = InputEventType::AxisMove;
        event.deviceId = m_deviceId;
        event.deviceType = m_deviceType;
        event.axis.axisId = static_cast<AxisID>(MouseAxis::Y);
        event.axis.value = m_positionY;
        event.axis.delta = m_deltaY;
        event.timestamp = std::chrono::steady_clock::now();
        add_event(event);
    }
}

void MouseDevice::handle_wheel(float delta) {
    m_wheelDelta += delta;
    
    InputEvent event;
    event.type = InputEventType::AxisMove;
    event.deviceId = m_deviceId;
    event.deviceType = m_deviceType;
    event.axis.axisId = static_cast<AxisID>(MouseAxis::Wheel);
    event.axis.value = m_wheelDelta;
    event.axis.delta = delta;
    event.timestamp = std::chrono::steady_clock::now();
    add_event(event);
}

void MouseDevice::handle_horizontal_wheel(float delta) {
    m_horizontalWheelDelta += delta;
    
    InputEvent event;
    event.type = InputEventType::AxisMove;
    event.deviceId = m_deviceId;
    event.deviceType = m_deviceType;
    event.axis.axisId = static_cast<AxisID>(MouseAxis::HorizontalWheel);
    event.axis.value = m_horizontalWheelDelta;
    event.axis.delta = delta;
    event.timestamp = std::chrono::steady_clock::now();
    add_event(event);
}

void MouseDevice::set_cursor_visible(bool visible) {
    m_cursorVisible = visible;
#ifdef _WIN32
    ShowCursor(visible ? TRUE : FALSE);
#endif
}

void MouseDevice::set_cursor_locked(bool locked) {
    m_cursorLocked = locked;
#ifdef _WIN32
    if (locked) {
        RECT rect;
        GetClientRect(GetForegroundWindow(), &rect);
        POINT topLeft = {rect.left, rect.top};
        POINT bottomRight = {rect.right, rect.bottom};
        ClientToScreen(GetForegroundWindow(), &topLeft);
        ClientToScreen(GetForegroundWindow(), &bottomRight);
        rect.left = topLeft.x;
        rect.top = topLeft.y;
        rect.right = bottomRight.x;
        rect.bottom = bottomRight.y;
        ClipCursor(&rect);
    } else {
        ClipCursor(nullptr);
    }
#endif
}

void MouseDevice::update_button_state(MouseButton button, bool pressed) {
    size_t index = static_cast<size_t>(button);
    if (index < m_buttonStates.size()) {
        m_buttonStates[index].pressed = pressed;
    }
}

void MouseDevice::reset_deltas() {
    m_deltaX = 0.0f;
    m_deltaY = 0.0f;
    m_wheelDelta = 0.0f;
    m_horizontalWheelDelta = 0.0f;
}

} // namespace CyberEngine::Input