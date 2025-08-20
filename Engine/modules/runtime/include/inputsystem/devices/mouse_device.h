#pragma once

#include "inputsystem/core/input_device.h"
#include <array>

namespace Cyber::Input {

class MouseDevice : public IInputDevice {
public:
    MouseDevice(DeviceID id);
    ~MouseDevice() override = default;
    
    bool initialize() override;
    void shutdown() override;
    void update(float deltaTime) override;
    void end_frame() override;

    bool is_connected() const override { return m_connected; }
    std::string get_device_name() const override { return "Mouse"; }
    
    ButtonState get_button_state(ButtonID button) const override;
    float get_axis_value(AxisID axis) const override;
    float get_axis_delta(AxisID axis) const override;

    bool is_button_pressed(ButtonID button) const override;
    bool is_button_just_pressed(ButtonID button) const override;
    bool is_button_just_released(ButtonID button) const override;

    // Platform-specific input handling
    void handle_button_down(MouseButton button);
    void handle_button_up(MouseButton button);
    void handle_move(float x, float y);
    void handle_wheel(float delta);
    void handle_horizontal_wheel(float delta);

    // Mouse state
    void get_position(float& x, float& y) const { x = m_positionX; y = m_positionY; }
    void get_delta(float& deltaX, float& deltaY) const { deltaX = m_deltaX; deltaY = m_deltaY; }
    float get_wheel_delta() const { return m_wheelDelta; }

    // Cursor control
    void set_cursor_visible(bool visible);
    void set_cursor_locked(bool locked);
    bool is_cursor_visible() const { return m_cursorVisible; }
    bool is_cursor_locked() const { return m_cursorLocked; }
    
private:
    struct ButtonStateInfo {
        bool pressed = false;
        bool released = false;
        bool justPressed = false;
        bool justReleased = false;
    };
    
    std::array<ButtonStateInfo, static_cast<size_t>(MouseButton::Count)> m_buttonStates;
    std::array<ButtonStateInfo, static_cast<size_t>(MouseButton::Count)> m_previousButtonStates;
    
    float m_positionX = 0.0f;
    float m_positionY = 0.0f;
    float m_previousX = 0.0f;
    float m_previousY = 0.0f;
    float m_deltaX = 0.0f;
    float m_deltaY = 0.0f;
    float m_wheelDelta = 0.0f;
    float m_horizontalWheelDelta = 0.0f;
    
    bool m_cursorVisible = true;
    bool m_cursorLocked = false;
    bool m_firstMove = true;
    
    void update_button_state(MouseButton button, bool pressed);
    void reset_deltas();
};

} // namespace CyberEngine::Input