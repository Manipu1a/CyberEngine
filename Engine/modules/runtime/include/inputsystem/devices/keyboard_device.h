#pragma once

#include "inputsystem/core/input_device.h"
#include <array>
#include <unordered_map>

namespace Cyber::Input {

class KeyboardDevice : public IInputDevice {
public:
    KeyboardDevice(DeviceID id);
    ~KeyboardDevice() override = default;
    
    bool initialize() override;
    void shutdown() override;
    void update(float deltaTime) override;
    void end_frame() override;
    
    bool is_connected() const override { return m_connected; }
    std::string get_device_name() const override { return "Keyboard"; }
    
    ButtonState get_button_state(ButtonID button) const override;
    float get_axis_value(AxisID axis) const override { return 0.0f; }
    float get_axis_delta(AxisID axis) const override { return 0.0f; }

    bool is_button_pressed(ButtonID button) const override;
    bool is_button_just_pressed(ButtonID button) const override;
    bool is_button_just_released(ButtonID button) const override;

    // Platform-specific input handling
    void handle_key_down(Key key);
    void handle_key_up(Key key);
    void handle_char(wchar_t character);

    // Text input
    bool is_text_input_active() const { return m_textInputActive; }
    void set_text_input_active(bool active) { m_textInputActive = active; }
    const std::wstring& get_text_input() const { return m_textBuffer; }
    void clear_text_input() { m_textBuffer.clear(); }
    
private:
    struct KeyStateInfo {
        bool pressed = false;
        bool justPressed = false;
        bool justReleased = false;
    };
    
    std::array<KeyStateInfo, static_cast<size_t>(Key::Count)> m_keyStates;
    std::array<KeyStateInfo, static_cast<size_t>(Key::Count)> m_previousKeyStates;
    
    bool m_textInputActive = false;
    std::wstring m_textBuffer;

    void update_key_state(Key key, bool pressed);
#ifdef _WIN32
    static std::unordered_map<int, Key> s_vkToKeyMap;
    static void initialize_key_map();
public:
    static Key virtual_key_to_key(int vk);
#endif
};

} // namespace CyberEngine::Input