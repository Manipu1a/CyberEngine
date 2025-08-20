#include "inputsystem/core/input_manager.h"
#include "inputsystem/core/input_context.h"
#include "inputsystem/devices/keyboard_device.h"
#include "inputsystem/devices/mouse_device.h"
#include <algorithm>
#include "inputsystem/platform/win32/Input_backend_win32.h"

namespace Cyber::Input {

InputManager* InputManager::instance = nullptr;
InputManager* InputManager::get_instance()
{
    if(instance == nullptr)
    {
        #if defined(_WINDOWS)
        instance = new InputBackendWin32();
        #endif
    }
    return instance;
}
bool InputManager::initialize(void* windowHandle) {
    m_windowHandle = windowHandle;
    
    // Create default devices
    m_keyboard = create_device<KeyboardDevice>();
    m_mouse = create_device<MouseDevice>();

    return true;
}

void InputManager::shutdown() {
    m_contextStack.clear();
    m_contexts.clear();
    
    for (auto& [id, device] : m_devices) {
        device->shutdown();
    }
    m_devices.clear();
    
    m_keyboard = nullptr;
    m_mouse = nullptr;
    m_gamepads.clear();
}

void InputManager::update(float deltaTime) {
    update_devices(deltaTime);
    process_events();
    update_context(deltaTime);

    m_mouse->end_frame();
}

void InputManager::remove_device(DeviceID id) {
    auto it = m_devices.find(id);
    if (it != m_devices.end()) {
        InputEvent event;
        event.type = InputEventType::DeviceDisconnected;
        event.deviceId = id;
        event.deviceType = it->second->get_device_type();
        event.timestamp = std::chrono::steady_clock::now();
        m_eventQueue.push(event);

        it->second->shutdown();
        m_devices.erase(it);
        
        // Update cached pointers
        if (m_keyboard && m_keyboard->get_device_id() == id) {
            m_keyboard = nullptr;
        }
        if (m_mouse && m_mouse->get_device_id() == id) {
            m_mouse = nullptr;
        }
        
        auto gamepadIt = std::find_if(m_gamepads.begin(), m_gamepads.end(),
            [id](IInputDevice* device) { return device->get_device_id() == id; });
        if (gamepadIt != m_gamepads.end()) {
            m_gamepads.erase(gamepadIt);
        }
    }
}

IInputDevice* InputManager::get_device(DeviceID id) {
    auto it = m_devices.find(id);
    return it != m_devices.end() ? it->second.get() : nullptr;
}

std::vector<IInputDevice*> InputManager::get_devices_by_type(DeviceType type) {
    std::vector<IInputDevice*> result;
    for (auto& [id, device] : m_devices) {
        if (device->get_device_type() == type) {
            result.push_back(device.get());
        }
    }
    return result;
}

InputContext* InputManager::create_context(const std::string& name) {
    auto context = std::make_unique<InputContext>(name, this);
    InputContext* ptr = context.get();
    m_contexts[name] = std::move(context);
    return ptr;
}

void InputManager::destroy_context(const std::string& name) {
    auto it = m_contexts.find(name);
    if (it != m_contexts.end()) {
        // Remove from stack if present
        auto stackIt = std::find(m_contextStack.begin(), m_contextStack.end(), it->second.get());
        if (stackIt != m_contextStack.end()) {
            m_contextStack.erase(stackIt);
        }
        m_contexts.erase(it);
    }
}

void InputManager::push_context(InputContext* context) {
    if (context) {
        m_contextStack.push_back(context);
    }
}

void InputManager::pop_context() {
    if (!m_contextStack.empty()) {
        m_contextStack.pop_back();
    }
}

InputContext* InputManager::get_active_context() {
    return m_contextStack.empty() ? nullptr : m_contextStack.back();
}

InputContext* InputManager::get_context(const std::string& name) {
    auto it = m_contexts.find(name);
    return it != m_contexts.end() ? it->second.get() : nullptr;
}

void InputManager::process_platform_event(void* nativeEvent) {
    // Platform-specific event processing
    // This will be implemented in platform-specific files
}

void InputManager::register_event_callback(InputCallback callback) {
    m_eventCallbacks.push_back(callback);
}

void InputManager::unregister_event_callback(InputCallback callback) {
    // Note: std::function doesn't support equality comparison
    // In production code, you might want to use a different approach
}

bool InputManager::is_key_pressed(Key key) const {
    if (m_keyboard) {
        return m_keyboard->is_button_pressed(static_cast<ButtonID>(key));
    }
    return false;
}

bool InputManager::is_key_just_pressed(Key key) const {
    if (m_keyboard) {
        return m_keyboard->is_button_just_pressed(static_cast<ButtonID>(key));
    }
    return false;
}

bool InputManager::is_key_just_released(Key key) const {
    if (m_keyboard) {
        return m_keyboard->is_button_just_released(static_cast<ButtonID>(key));
    }
    return false;
}

bool InputManager::is_mouse_button_pressed(MouseButton button) const {
    if (m_mouse) {
        return m_mouse->is_button_pressed(static_cast<ButtonID>(button));
    }
    return false;
}

bool InputManager::is_mouse_button_just_pressed(MouseButton button) const {
    if (m_mouse) {
        return m_mouse->is_button_just_pressed(static_cast<ButtonID>(button));
    }
    return false;
}

bool InputManager::is_mouse_button_just_released(MouseButton button) const {
    if (m_mouse) {
        return m_mouse->is_button_just_released(static_cast<ButtonID>(button));
    }
    return false;
}

void InputManager::get_mouse_position(float& x, float& y) const {
    if (m_mouse) {
        x = m_mouse->get_axis_value(static_cast<AxisID>(MouseAxis::X));
        y = m_mouse->get_axis_value(static_cast<AxisID>(MouseAxis::Y));
    } else {
        x = y = 0.0f;
    }
}

void InputManager::get_mouse_delta(float& deltaX, float& deltaY) const {
    if (m_mouse) {
        deltaX = m_mouse->get_axis_delta(static_cast<AxisID>(MouseAxis::X));
        deltaY = m_mouse->get_axis_delta(static_cast<AxisID>(MouseAxis::Y));
    } else {
        deltaX = deltaY = 0.0f;
    }
}

float InputManager::get_mouse_wheel() const {
    if (m_mouse) {
        return m_mouse->get_axis_value(static_cast<AxisID>(MouseAxis::Wheel));
    }
    return 0.0f;
}

bool InputManager::is_gamepad_connected(int index) const {
    return index >= 0 && index < static_cast<int>(m_gamepads.size()) &&
           m_gamepads[index] && m_gamepads[index]->is_connected();
}

bool InputManager::is_gamepad_button_pressed(GamepadButton button, int index) const {
    if (is_gamepad_connected(index)) {
        return m_gamepads[index]->is_button_pressed(static_cast<ButtonID>(button));
    }
    return false;
}

float InputManager::get_gamepad_axis(GamepadAxis axis, int index) const {
    if (is_gamepad_connected(index)) {
        return m_gamepads[index]->get_axis_value(static_cast<AxisID>(axis));
    }
    return 0.0f;
}

void InputManager::set_gamepad_vibration(float leftMotor, float rightMotor, int index) {
    if (is_gamepad_connected(index)) {
        m_gamepads[index]->set_vibration(leftMotor, rightMotor);
    }
}

void InputManager::process_events() {
    while (!m_eventQueue.empty()) {
        InputEvent event = m_eventQueue.front();
        m_eventQueue.pop();
        
        // Notify callbacks
        for (const auto& callback : m_eventCallbacks) {
            callback(event);
        }
        
        // Process in active context
        if (auto* context = get_active_context()) {
            context->process_event(event);
        }
    }
}

void InputManager::update_devices(float deltaTime) {
    for (auto& [id, device] : m_devices) {
        device->update(deltaTime);

        // Collect events from devices
        for (const auto& event : device->get_events()) {
            m_eventQueue.push(event);
        }
        device->clear_events();
    }
}

void InputManager::update_context(float deltaTime)
{
    if (auto* context = get_active_context()) {
            context->update(deltaTime);
    }
}
} // namespace CyberEngine::Input