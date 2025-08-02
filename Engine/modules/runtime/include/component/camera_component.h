#pragma once 
#include "platform/configure.h"
#include "math/basic_math.hpp"
#include <gainput/gainput.h>
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
    CameraComponent();
    
    void update();
private:
    float yaw = 0.0f;
    float pitch = 0.0f;
    float roll = 0.0f;

    Math::Quaternion<float> rotation;
    gainput::InputManager* input_manager = nullptr;
    gainput::InputMap* map;

};

CYBER_END_NAMESPACE
CYBER_END_NAMESPACE
