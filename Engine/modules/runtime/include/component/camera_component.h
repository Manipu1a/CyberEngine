#pragma once
#include "platform/configure.h"
#include "math/basic_math.hpp"
#include "inputsystem/core/input_manager.h"
#include "cyber_runtime.config.h"
#include "component/primitive.h"

CYBER_BEGIN_NAMESPACE(Cyber)
CYBER_BEGIN_NAMESPACE(Component)

struct CYBER_RUNTIME_API CameraAttribs
{
    float4 camera_position;
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

// CameraComponent is a Primitive that stores its transform via Primitive::position
// (as the eye point) + yaw/pitch used to drive Primitive::rotation each frame.
// Projection parameters (fov/near/far) live here; the view matrix is recomputed
// in update().
class CYBER_RUNTIME_API CameraComponent : public Primitive
{
public:
    CameraComponent();
    explicit CameraComponent(float3 initial_position);
    ~CameraComponent() override = default;

    const char*      type_name() const override { return "CameraComponent"; }
    Scope<Primitive> clone() const override;

    void update(float deltaTime = 0.0f);

    void mouse_wheel_move(float value);

    // World-space forward vector derived from yaw/pitch.
    float3 get_forward() const;
    float3 get_right() const;

    // Transform accessors use the base-class fields.
    float3 get_camera_position() const { return position; }
    void   set_camera_position(const float3& p) { position = p; }
    Math::Quaternion<float> get_rotation() const { return rotation; }

    float4x4 get_view_matrix() const { return m_view_matrix; }
    void     set_view_matrix(const float4x4& m) { m_view_matrix = m; }
    float4x4 get_projection_matrix() const { return m_projection_matrix; }
    void     set_projection_matrix(const float4x4& m) { m_projection_matrix = m; }

    float get_yaw() const { return yaw; }
    float get_pitch() const { return pitch; }
    void  set_yaw(float v) { yaw = v; }
    void  set_pitch(float v) { pitch = v; }

    // Projection parameters (serialized).
    float fov_deg = 60.0f;
    float near_z  = 0.1f;
    float far_z   = 1000.0f;

private:
    void register_input_bindings();

    float yaw = 0.0f;
    float pitch = 0.0f;
    float roll = 0.0f;

    float last_mouse_x = 0.0f;
    float last_mouse_y = 0.0f;
    bool  dragging       = false;
    float mouse_sensitivity = 0.005f;   // radians per pixel
    float move_speed        = 10.0f;    // world units per second
    Input::InputContext* m_gameContext = nullptr;

    float4x4 m_view_matrix;
    float4x4 m_projection_matrix;
};

CYBER_END_NAMESPACE
CYBER_END_NAMESPACE
