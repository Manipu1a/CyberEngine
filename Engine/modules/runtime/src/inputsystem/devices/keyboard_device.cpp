#include "inputsystem/devices/keyboard_device.h"
#include <chrono>

#ifdef _WIN32
#include <Windows.h>
#endif

namespace CyberEngine::Input {

#ifdef _WIN32
std::unordered_map<int, Key> KeyboardDevice::s_vkToKeyMap;
#endif

KeyboardDevice::KeyboardDevice(DeviceID id)
    : IInputDevice(id, DeviceType::Keyboard) {
#ifdef _WIN32
    initialize_key_map();
#endif
}

bool KeyboardDevice::initialize() {
    m_connected = true;
    return true;
}

void KeyboardDevice::shutdown() {
    m_connected = false;
}

void KeyboardDevice::update(float deltaTime) {
    // Update just pressed/released states
    for (size_t i = 0; i < m_keyStates.size(); ++i) {
        m_keyStates[i].justPressed = m_keyStates[i].pressed && !m_previousKeyStates[i].pressed;
        m_keyStates[i].justReleased = !m_keyStates[i].pressed && m_previousKeyStates[i].pressed;
    }
    
    m_previousKeyStates = m_keyStates;
}

ButtonState KeyboardDevice::get_button_state(ButtonID button) const {
    if (button < static_cast<ButtonID>(Key::Count)) {
        return m_keyStates[button].pressed ? ButtonState::Pressed : ButtonState::Released;
    }
    return ButtonState::Released;
}

bool KeyboardDevice::is_button_pressed(ButtonID button) const {
    if (button < static_cast<ButtonID>(Key::Count)) {
        return m_keyStates[button].pressed;
    }
    return false;
}

bool KeyboardDevice::is_button_just_pressed(ButtonID button) const {
    if (button < static_cast<ButtonID>(Key::Count)) {
        return m_keyStates[button].justPressed;
    }
    return false;
}

bool KeyboardDevice::is_button_just_released(ButtonID button) const {
    if (button < static_cast<ButtonID>(Key::Count)) {
        return m_keyStates[button].justReleased;
    }
    return false;
}

void KeyboardDevice::handle_key_down(Key key) {
    if (key != Key::Unknown && static_cast<size_t>(key) < m_keyStates.size()) {
        update_key_state(key, true);

        InputEvent event;
        event.type = InputEventType::ButtonDown;
        event.deviceId = m_deviceId;
        event.deviceType = m_deviceType;
        event.button.buttonId = static_cast<ButtonID>(key);
        event.button.state = ButtonState::Pressed;
        event.timestamp = std::chrono::steady_clock::now();
        add_event(event);
    }
}

void KeyboardDevice::handle_key_up(Key key) {
    if (key != Key::Unknown && static_cast<size_t>(key) < m_keyStates.size()) {
        update_key_state(key, false);

        InputEvent event;
        event.type = InputEventType::ButtonUp;
        event.deviceId = m_deviceId;
        event.deviceType = m_deviceType;
        event.button.buttonId = static_cast<ButtonID>(key);
        event.button.state = ButtonState::Released;
        event.timestamp = std::chrono::steady_clock::now();
        add_event(event);
    }
}

void KeyboardDevice::handle_char(wchar_t character) {
    if (m_textInputActive) {
        m_textBuffer += character;
    }
}

void KeyboardDevice::update_key_state(Key key, bool pressed) {
    size_t index = static_cast<size_t>(key);
    if (index < m_keyStates.size()) {
        m_keyStates[index].pressed = pressed;
    }
}

#ifdef _WIN32
void KeyboardDevice::initialize_key_map() {
    if (!s_vkToKeyMap.empty()) return;
    
    // Letters
    for (int i = 0; i < 26; ++i) {
        s_vkToKeyMap['A' + i] = static_cast<Key>(static_cast<int>(Key::A) + i);
    }
    
    // Numbers
    for (int i = 0; i < 10; ++i) {
        s_vkToKeyMap['0' + i] = static_cast<Key>(static_cast<int>(Key::Num0) + i);
    }
    
    // Function keys
    for (int i = 0; i < 15; ++i) {
        s_vkToKeyMap[VK_F1 + i] = static_cast<Key>(static_cast<int>(Key::F1) + i);
    }
    
    // Special keys
    s_vkToKeyMap[VK_ESCAPE] = Key::Escape;
    s_vkToKeyMap[VK_TAB] = Key::Tab;
    s_vkToKeyMap[VK_CAPITAL] = Key::CapsLock;
    s_vkToKeyMap[VK_SHIFT] = Key::Shift;
    s_vkToKeyMap[VK_LSHIFT] = Key::Shift;
    s_vkToKeyMap[VK_RSHIFT] = Key::Shift;
    s_vkToKeyMap[VK_CONTROL] = Key::Control;
    s_vkToKeyMap[VK_LCONTROL] = Key::Control;
    s_vkToKeyMap[VK_RCONTROL] = Key::Control;
    s_vkToKeyMap[VK_MENU] = Key::Alt;
    s_vkToKeyMap[VK_LMENU] = Key::Alt;
    s_vkToKeyMap[VK_RMENU] = Key::Alt;
    s_vkToKeyMap[VK_SPACE] = Key::Space;
    s_vkToKeyMap[VK_RETURN] = Key::Enter;
    s_vkToKeyMap[VK_BACK] = Key::Backspace;
    s_vkToKeyMap[VK_DELETE] = Key::Delete;
    
    // Arrow keys
    s_vkToKeyMap[VK_UP] = Key::Up;
    s_vkToKeyMap[VK_DOWN] = Key::Down;
    s_vkToKeyMap[VK_LEFT] = Key::Left;
    s_vkToKeyMap[VK_RIGHT] = Key::Right;
    
    // Navigation keys
    s_vkToKeyMap[VK_INSERT] = Key::Insert;
    s_vkToKeyMap[VK_HOME] = Key::Home;
    s_vkToKeyMap[VK_END] = Key::End;
    s_vkToKeyMap[VK_PRIOR] = Key::PageUp;
    s_vkToKeyMap[VK_NEXT] = Key::PageDown;
    
    // System keys
    s_vkToKeyMap[VK_SNAPSHOT] = Key::PrintScreen;
    s_vkToKeyMap[VK_SCROLL] = Key::ScrollLock;
    s_vkToKeyMap[VK_PAUSE] = Key::Pause;
    
    // Numpad
    for (int i = 0; i < 10; ++i) {
        s_vkToKeyMap[VK_NUMPAD0 + i] = static_cast<Key>(static_cast<int>(Key::Numpad0) + i);
    }
    s_vkToKeyMap[VK_ADD] = Key::NumpadAdd;
    s_vkToKeyMap[VK_SUBTRACT] = Key::NumpadSubtract;
    s_vkToKeyMap[VK_MULTIPLY] = Key::NumpadMultiply;
    s_vkToKeyMap[VK_DIVIDE] = Key::NumpadDivide;
    s_vkToKeyMap[VK_DECIMAL] = Key::NumpadDecimal;
    s_vkToKeyMap[VK_NUMLOCK] = Key::NumpadLock;
}

Key KeyboardDevice::virtual_key_to_key(int vk) {
    auto it = s_vkToKeyMap.find(vk);
    return it != s_vkToKeyMap.end() ? it->second : Key::Unknown;
}
#endif

} // namespace CyberEngine::Input