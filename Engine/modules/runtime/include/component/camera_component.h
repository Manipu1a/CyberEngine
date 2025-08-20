#pragma once 
#include "platform/configure.h"
#include "math/basic_math.hpp"
#include "inputsystem/core/input_manager.h"
#include "cyber_runtime.config.h"

CYBER_BEGIN_NAMESPACE(Cyber)
CYBER_BEGIN_NAMESPACE(Component)

struct CYBER_RUNTIME_API CameraAttribs
{
    float4 camera_position;
    //float4x4 view_matrix;
    //float4x4 projection_matrix;
    //float4x4 view_projection_matrix;
    float4x4 inverse_view_projection_matrix;
};

enum CameraMovement
{
    Forward = 0,
    Backward,
    Left,
    Right,
    Up,
    Down
};

class CYBER_RUNTIME_API CameraComponent
{
public:
    CameraComponent(float3 _camera_position);
    
    void update();

    void move_forward(float value);
    void move_backward(float value);
    void move_left(float value);
    void move_right(float value);
    void mouse_wheel_move(float value);

    float3 get_camera_position() const { return camera_position; }
    void set_camera_position(const float3& position) { camera_position = position; }
    Math::Quaternion<float> get_rotation() const { return rotation; }
private:
    float yaw = 0.0f;
    float pitch = 0.0f;
    float roll = 0.0f;

    float last_mouse_x = 0.0f;
    float last_mouse_y = 0.0f;
    float move_speed = 0.05f;
    float3 camera_position = { 0.0f, 0.0f, 5.0f };
    Math::Quaternion<float> rotation;
    Input::InputContext* m_gameContext = nullptr;
};

CYBER_END_NAMESPACE
CYBER_END_NAMESPACE
