#pragma once

#include "input_types.h"
#include <string>
#include <vector>

namespace CyberEngine::Input {

class IInputDevice {
public:
    IInputDevice(DeviceID id, DeviceType type) 
        : m_deviceId(id), m_deviceType(type) {}
    
    virtual ~IInputDevice() = default;
    
    virtual bool initialize() = 0;
    virtual void shutdown() = 0;
    virtual void update(float deltaTime) = 0;
    
    virtual bool is_connected() const = 0;
    virtual std::string get_device_name() const = 0;

    virtual ButtonState get_button_state(ButtonID button) const = 0;
    virtual float get_axis_value(AxisID axis) const = 0;
    virtual float get_axis_delta(AxisID axis) const = 0;

    virtual bool is_button_pressed(ButtonID button) const = 0;
    virtual bool is_button_just_pressed(ButtonID button) const = 0;
    virtual bool is_button_just_released(ButtonID button) const = 0;

    virtual void set_vibration(float leftMotor, float rightMotor) {}

    DeviceID get_device_id() const { return m_deviceId; }
    DeviceType get_device_type() const { return m_deviceType; }

    const std::vector<InputEvent>& get_events() const { return m_events; }
    void clear_events() { m_events.clear(); }

protected:
    void add_event(const InputEvent& event) {
        m_events.push_back(event);
    }
    
    DeviceID m_deviceId;
    DeviceType m_deviceType;
    std::vector<InputEvent> m_events;
    bool m_connected = false;
};

} // namespace CyberEngine::Input