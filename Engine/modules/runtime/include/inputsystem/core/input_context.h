#pragma once

#include "input_types.h"
#include <string>
#include <unordered_map>
#include <vector>

namespace CyberEngine::Input {

class InputManager;

class InputAction {
public:
    InputAction(ActionID id, const std::string& name)
        : m_id(id), m_name(name) {}
    
    void add_binding(const InputBinding& binding) {
        m_bindings.push_back(binding);
    }
    
    void remove_binding(size_t index) {
        if (index < m_bindings.size()) {
            m_bindings.erase(m_bindings.begin() + index);
        }
    }

    void clear_bindings() {
        m_bindings.clear();
    }

    void set_callback(ActionCallback callback) {
        m_callback = callback;
    }

    void execute(float value) {
        if (m_callback) {
            m_callback(value);
        }
        m_value = value;
    }

    ActionID get_id() const { return m_id; }
    const std::string& get_name() const { return m_name; }
    const std::vector<InputBinding>& get_bindings() const { return m_bindings; }
    float get_value() const { return m_value; }

private:
    ActionID m_id;
    std::string m_name;
    std::vector<InputBinding> m_bindings;
    ActionCallback m_callback;
    float m_value = 0.0f;
};

class InputContext {
public:
    InputContext(const std::string& name, InputManager* manager)
        : m_name(name), m_manager(manager) {}
    
    // Action management
    InputAction* create_action(ActionID id, const std::string& name);
    void destroy_action(ActionID id);
    InputAction* get_action(ActionID id);
    InputAction* get_action(const std::string& name);
    
    // Binding management
    void bind_action(ActionID actionId, DeviceType deviceType, ButtonID buttonId, float scale = 1.0f);
    void bind_axis(ActionID actionId, DeviceType deviceType, AxisID axisId, float scale = 1.0f);
    void unbind_action(ActionID actionId);
    
    // Action queries
    bool is_action_pressed(ActionID actionId) const;
    bool is_action_just_pressed(ActionID actionId) const;
    bool is_action_just_released(ActionID actionId) const;
    float get_action_value(ActionID actionId) const;
    
    // Context properties
    void set_enabled(bool enabled) { m_enabled = enabled; }
    bool is_enabled() const { return m_enabled; }

    void set_priority(float priority) { m_priority = priority; }
    float get_priority() const { return m_priority; }

    const std::string& get_name() const { return m_name; }
    
    // Event processing
    void process_event(const InputEvent& event);
    void update(float deltaTime);

private:
    std::string m_name;
    InputManager* m_manager;
    std::unordered_map<ActionID, std::unique_ptr<InputAction>> m_actions;
    std::unordered_map<std::string, ActionID> m_actionNameMap;
    
    bool m_enabled = true;
    float m_priority = 0.0f;
    
    // State tracking for just pressed/released
    std::unordered_map<ActionID, ButtonState> m_previousStates;
    std::unordered_map<ActionID, ButtonState> m_currentStates;

    void update_action_state(ActionID actionId, const InputEvent& event);
    float calculate_action_value(InputAction* action) const;
};

} // namespace CyberEngine::Input