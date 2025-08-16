#include "inputsystem/core/input_context.h"
#include "inputsystem/core/input_manager.h"
#include <algorithm>

namespace CyberEngine::Input {

InputAction* InputContext::create_action(ActionID id, const std::string& name) {
    auto action = std::make_unique<InputAction>(id, name);
    InputAction* ptr = action.get();
    m_actions[id] = std::move(action);
    m_actionNameMap[name] = id;
    return ptr;
}

void InputContext::destroy_action(ActionID id) {
    auto it = m_actions.find(id);
    if (it != m_actions.end()) {
        // Remove from name map
        auto nameIt = std::find_if(m_actionNameMap.begin(), m_actionNameMap.end(),
            [id](const auto& pair) { return pair.second == id; });
        if (nameIt != m_actionNameMap.end()) {
            m_actionNameMap.erase(nameIt);
        }
        
        m_actions.erase(it);
        m_previousStates.erase(id);
        m_currentStates.erase(id);
    }
}

InputAction* InputContext::get_action(ActionID id) {
    auto it = m_actions.find(id);
    return it != m_actions.end() ? it->second.get() : nullptr;
}

InputAction* InputContext::get_action(const std::string& name) {
    auto it = m_actionNameMap.find(name);
    if (it != m_actionNameMap.end()) {
        return get_action(it->second);
    }
    return nullptr;
}

void InputContext::bind_action(ActionID actionId, DeviceType deviceType, ButtonID buttonId, float scale) {
    if (auto* action = get_action(actionId)) {
        InputBinding binding;
        binding.deviceType = deviceType;
        binding.deviceId = INVALID_DEVICE_ID; // Will match any device of this type
        binding.buttonId = buttonId;
        binding.scale = scale;
        action->add_binding(binding);
    }
}

void InputContext::bind_axis(ActionID actionId, DeviceType deviceType, AxisID axisId, float scale) {
    if (auto* action = get_action(actionId)) {
        InputBinding binding;
        binding.deviceType = deviceType;
        binding.deviceId = INVALID_DEVICE_ID;
        binding.buttonId = axisId; // Axes use the same ID space as buttons
        binding.scale = scale;
        action->add_binding(binding);
    }
}

void InputContext::unbind_action(ActionID actionId) {
    if (auto* action = get_action(actionId)) {
        action->clear_bindings();
    }
}

bool InputContext::is_action_pressed(ActionID actionId) const {
    auto it = m_currentStates.find(actionId);
    return it != m_currentStates.end() && 
           (it->second == ButtonState::Pressed || it->second == ButtonState::Held);
}

bool InputContext::is_action_just_pressed(ActionID actionId) const {
    auto currIt = m_currentStates.find(actionId);
    auto prevIt = m_previousStates.find(actionId);
    
    bool currentPressed = currIt != m_currentStates.end() && 
                         (currIt->second == ButtonState::Pressed || currIt->second == ButtonState::Held);
    bool previousPressed = prevIt != m_previousStates.end() && 
                          (prevIt->second == ButtonState::Pressed || prevIt->second == ButtonState::Held);
    
    return currentPressed && !previousPressed;
}

bool InputContext::is_action_just_released(ActionID actionId) const {
    auto currIt = m_currentStates.find(actionId);
    auto prevIt = m_previousStates.find(actionId);
    
    bool currentPressed = currIt != m_currentStates.end() && 
                         (currIt->second == ButtonState::Pressed || currIt->second == ButtonState::Held);
    bool previousPressed = prevIt != m_previousStates.end() && 
                          (prevIt->second == ButtonState::Pressed || prevIt->second == ButtonState::Held);
    
    return !currentPressed && previousPressed;
}

float InputContext::get_action_value(ActionID actionId) const {
    auto it = m_actions.find(actionId);
    return it != m_actions.end() ? it->second->get_value() : 0.0f;
}

void InputContext::process_event(const InputEvent& event) {
    if (!m_enabled) return;
    
    // Check all actions for matching bindings
    for (auto& [actionId, action] : m_actions) {
        update_action_state(actionId, event);
    }
}

void InputContext::update(float deltaTime) {
    if (!m_enabled) return;
    
    // Update previous states
    m_previousStates = m_currentStates;
    
    // Calculate and execute action values
    for (auto& [actionId, action] : m_actions) {
        float value = calculate_action_value(action.get());
        action->execute(value);
        
        // Update current state based on value
        if (std::abs(value) > 0.01f) {
            m_currentStates[actionId] = ButtonState::Held;
        } else {
            m_currentStates[actionId] = ButtonState::Released;
        }
    }
}

void InputContext::update_action_state(ActionID actionId, const InputEvent& event) {
    auto* action = get_action(actionId);
    if (!action) return;
    
    // Check if this event matches any of the action's bindings
    for (const auto& binding : action->get_bindings()) {
        bool matches = false;
        
        if (binding.deviceType == event.deviceType) {
            if (binding.deviceId == INVALID_DEVICE_ID || binding.deviceId == event.deviceId) {
                if (event.type == InputEventType::ButtonDown || event.type == InputEventType::ButtonUp) {
                    matches = (binding.buttonId == event.button.buttonId);
                } else if (event.type == InputEventType::AxisMove) {
                    matches = (binding.buttonId == event.axis.axisId);
                }
            }
        }
        
        if (matches) {
            // Update state based on event type
            if (event.type == InputEventType::ButtonDown) {
                m_currentStates[actionId] = ButtonState::Pressed;
            } else if (event.type == InputEventType::ButtonUp) {
                m_currentStates[actionId] = ButtonState::Released;
            }
            break;
        }
    }
}

float InputContext::calculate_action_value(InputAction* action) const {
    if (!action || !m_manager) return 0.0f;
    
    float maxValue = 0.0f;

    for (const auto& binding : action->get_bindings()) {
        // Get devices of the specified type
        auto devices = m_manager->get_devices_by_type(binding.deviceType);

        for (auto* device : devices) {
            if (binding.deviceId != INVALID_DEVICE_ID && device->get_device_id() != binding.deviceId) {
                continue;
            }
            
            float value = 0.0f;
            
            // Check if it's a button or axis
            if (binding.buttonId < 1000) { // Assuming button IDs are < 1000
                if (device->is_button_pressed(binding.buttonId)) {
                    value = 1.0f;
                }
            } else { // Axis
                value = device->get_axis_value(binding.buttonId - 1000);
            }
            
            // Apply scale and dead zone
            value *= binding.scale;
            if (std::abs(value) < binding.deadZone) {
                value = 0.0f;
            }
            
            // Use maximum value from all bindings
            if (std::abs(value) > std::abs(maxValue)) {
                maxValue = value;
            }
        }
    }
    
    return maxValue;
}

} // namespace CyberEngine::Input