#pragma once

#include "input_types.h"
#include "input_device.h"
#include "input_context.h"
#include <memory>
#include <vector>
#include <unordered_map>
#include <queue>
#include <mutex>

namespace CyberEngine::Input {

class InputManager {
public:
    static InputManager& get_instance() {
        static InputManager instance;
        return instance;
    }

    bool initialize(void* windowHandle = nullptr);
    void shutdown();
    void update(float deltaTime);

    // Device management
    template<typename T, typename... Args>
    T* create_device(Args&&... args) {
        static_assert(std::is_base_of_v<IInputDevice, T>, "T must derive from IInputDevice");
        
        DeviceID id = m_nextDeviceId++;
        auto device = std::make_unique<T>(id, std::forward<Args>(args)...);
        T* ptr = device.get();
        
        if (device->initialize()) {
            m_devices[id] = std::move(device);
            
            InputEvent event;
            event.type = InputEventType::DeviceConnected;
            event.deviceId = id;
            event.deviceType = ptr->get_device_type();
            event.timestamp = std::chrono::steady_clock::now();
            m_eventQueue.push(event);
            
            return ptr;
        }
        
        return nullptr;
    }

    void remove_device(DeviceID id);
    IInputDevice* get_device(DeviceID id);
    std::vector<IInputDevice*> get_devices_by_type(DeviceType type);

    // Context management
    InputContext* create_context(const std::string& name);
    void destroy_context(const std::string& name);
    void push_context(InputContext* context);
    void pop_context();
    InputContext* get_active_context();
    InputContext* get_context(const std::string& name);

    // Event handling
    virtual void process_platform_event(void* nativeEvent);
    void register_event_callback(InputCallback callback);
    void unregister_event_callback(InputCallback callback);

    // Quick access methods
    bool is_key_pressed(Key key) const;
    bool is_key_just_pressed(Key key) const;
    bool is_key_just_released(Key key) const;

    bool is_mouse_button_pressed(MouseButton button) const;
    bool is_mouse_button_just_pressed(MouseButton button) const;
    bool is_mouse_button_just_released(MouseButton button) const;

    void get_mouse_position(float& x, float& y) const;
    void get_mouse_delta(float& deltaX, float& deltaY) const;
    float get_mouse_wheel() const;

    bool is_gamepad_connected(int index = 0) const;
    bool is_gamepad_button_pressed(GamepadButton button, int index = 0) const;
    float get_gamepad_axis(GamepadAxis axis, int index = 0) const;
    void set_gamepad_vibration(float leftMotor, float rightMotor, int index = 0);
    
    // Window info
    void set_window_size(int width, int height) {
        m_windowWidth = width;
        m_windowHeight = height;
    }
    int get_window_width() const { return m_windowWidth; }
    int get_window_height() const { return m_windowHeight; }

protected:
    InputManager() = default;
    ~InputManager() = default;
    InputManager(const InputManager&) = delete;
    InputManager& operator=(const InputManager&) = delete;
    
    void process_events();
    void update_devices(float deltaTime);

    std::unordered_map<DeviceID, std::unique_ptr<IInputDevice>> m_devices;
    std::unordered_map<std::string, std::unique_ptr<InputContext>> m_contexts;
    std::vector<InputContext*> m_contextStack;
    std::queue<InputEvent> m_eventQueue;
    std::vector<InputCallback> m_eventCallbacks;
    
    DeviceID m_nextDeviceId = 1;
    void* m_windowHandle = nullptr;
    int m_windowWidth = 0;
    int m_windowHeight = 0;
    
    mutable std::mutex m_mutex;
    
    // Cache common devices for quick access
    IInputDevice* m_keyboard = nullptr;
    IInputDevice* m_mouse = nullptr;
    std::vector<IInputDevice*> m_gamepads;
};

} // namespace CyberEngine::Input