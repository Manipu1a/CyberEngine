#include "component/camera_component.h"
#include "application/application.h"
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include <cstring>

CYBER_BEGIN_NAMESPACE(Cyber)
CYBER_BEGIN_NAMESPACE(Component)

enum GameActions : Input::ActionID {
    ACTION_MOVE_FORWARD = 0,
    ACTION_MOVE_BACKWARD,
    ACTION_MOVE_LEFT,
    ACTION_MOVE_RIGHT,
    ACTION_MOVE_UP,
    ACTION_MOVE_DOWN,
    ACTION_MOUSE_LEFT,
    ACTION_MOUSE_WHEEL,
    ACTION_LOOK_X,
    ACTION_LOOK_Y,
    ACTION_PAUSE
};

CameraComponent::CameraComponent() : Primitive(ComponentType::Camera)
{
    position = float3(0.0f, 0.0f, 5.0f);
    register_input_bindings();
}

CameraComponent::CameraComponent(float3 initial_position) : Primitive(ComponentType::Camera)
{
    position = initial_position;
    register_input_bindings();
}

Scope<Primitive> CameraComponent::clone() const
{
    auto copy = Scope<CameraComponent>(new CameraComponent());
    copy->position      = position;
    copy->rotation      = rotation;
    copy->scale         = scale;
    copy->enabled       = enabled;
    copy->name          = name;
    copy->yaw           = yaw;
    copy->pitch         = pitch;
    copy->roll          = roll;
    copy->fov_deg       = fov_deg;
    copy->near_z        = near_z;
    copy->far_z         = far_z;
    copy->m_projection_matrix = m_projection_matrix;
    return Scope<Primitive>(copy.release());
}

void CameraComponent::register_input_bindings()
{
    yaw = 0.0f;
    pitch = 0.0f;
    roll = 0.0f;

    auto inputMgr = Input::InputManager::get_instance();
    if (!inputMgr)
        return;

    // Reuse an already-registered "Game" context if another CameraComponent
    // was constructed earlier — creating two contexts with the same name
    // would double-dispatch the same input event.
    m_gameContext = inputMgr->get_context("Game");
    if (m_gameContext)
        return;

    m_gameContext = inputMgr->create_context("Game");

    m_gameContext->create_action(ACTION_MOVE_FORWARD,  "MoveForward");
    m_gameContext->create_action(ACTION_MOVE_BACKWARD, "MoveBackward");
    m_gameContext->create_action(ACTION_MOVE_LEFT,     "MoveLeft");
    m_gameContext->create_action(ACTION_MOVE_RIGHT,    "MoveRight");
    m_gameContext->create_action(ACTION_MOVE_UP,       "MoveUp");
    m_gameContext->create_action(ACTION_MOVE_DOWN,     "MoveDown");
    m_gameContext->create_action(ACTION_MOUSE_LEFT,    "MouseLeft");
    auto* mouse_wheel = m_gameContext->create_action(ACTION_MOUSE_WHEEL, "MouseWheel");

    m_gameContext->bind_action(ACTION_MOVE_FORWARD,  Input::DeviceType::Keyboard, static_cast<Input::ButtonID>(Input::Key::W));
    m_gameContext->bind_action(ACTION_MOVE_BACKWARD, Input::DeviceType::Keyboard, static_cast<Input::ButtonID>(Input::Key::S));
    m_gameContext->bind_action(ACTION_MOVE_LEFT,     Input::DeviceType::Keyboard, static_cast<Input::ButtonID>(Input::Key::A));
    m_gameContext->bind_action(ACTION_MOVE_RIGHT,    Input::DeviceType::Keyboard, static_cast<Input::ButtonID>(Input::Key::D));
    m_gameContext->bind_action(ACTION_MOVE_UP,       Input::DeviceType::Keyboard, static_cast<Input::ButtonID>(Input::Key::E));
    m_gameContext->bind_action(ACTION_MOVE_DOWN,     Input::DeviceType::Keyboard, static_cast<Input::ButtonID>(Input::Key::Q));
    m_gameContext->bind_action(ACTION_MOUSE_LEFT,    Input::DeviceType::Mouse,    static_cast<Input::ButtonID>(Input::MouseButton::Left));
    m_gameContext->bind_action(ACTION_MOUSE_WHEEL,   Input::DeviceType::Mouse,    static_cast<Input::ButtonID>(Input::MouseAxis::Wheel) + 1000);

    mouse_wheel->set_callback([this](float v){ mouse_wheel_move(v); });

    inputMgr->push_context(m_gameContext);
}

float3 CameraComponent::get_forward() const
{
    const float cp = std::cos(pitch);
    const float sp = std::sin(pitch);
    const float cy = std::cos(yaw);
    const float sy = std::sin(yaw);
    return float3(cp * sy, sp, cp * cy);
}

float3 CameraComponent::get_right() const
{
    // Right is forward rotated -90deg around world up (Y).
    const float cy = std::cos(yaw);
    const float sy = std::sin(yaw);
    return float3(cy, 0.0f, -sy);
}

void CameraComponent::update(float deltaTime)
{
    auto inputMgr = Input::InputManager::get_instance();
    if (!inputMgr || !m_gameContext)
        return;

    float x = 0, y = 0;
    inputMgr->get_mouse_position(x, y);

    bool isMouseInViewport = false;
    if (ImGui::GetCurrentContext() != nullptr)
    {
        ImGuiWindow* viewportWindow = ImGui::FindWindowByName("Viewport");
        if (viewportWindow != nullptr && ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow))
        {
            ImGuiWindow* hoveredWindow = ImGui::GetCurrentContext()->HoveredWindow;
            if (hoveredWindow != nullptr)
            {
                isMouseInViewport = (hoveredWindow == viewportWindow) ||
                                   (hoveredWindow->ParentWindow == viewportWindow) ||
                                   (strcmp(hoveredWindow->Name, "ViewportChild") == 0);
            }
        }
    }
    else
    {
        // No editor context — treat the whole window as the viewport.
        isMouseInViewport = true;
    }

    // Left-mouse drag rotates yaw (around Y) and pitch (around X).
    // Track a `dragging` edge to avoid applying the inter-frame mouse motion
    // accumulated while the button was up as a single giant rotation on click.
    const bool mouseDown = m_gameContext->is_action_pressed(ACTION_MOUSE_LEFT);
    if (mouseDown && isMouseInViewport)
    {
        if (dragging)
        {
            const float dx = x - last_mouse_x;
            const float dy = y - last_mouse_y;
            // dx > 0 (mouse right) → yaw toward +X; dy > 0 (mouse down, screen-Y) → pitch down.
            yaw   += dx * mouse_sensitivity;
            pitch -= dy * mouse_sensitivity;
        }
        dragging = true;
    }
    else
    {
        dragging = false;
    }
    last_mouse_x = x;
    last_mouse_y = y;

    const float pitch_limit = 1.55334f; // ~89 degrees
    if (pitch >  pitch_limit) pitch =  pitch_limit;
    if (pitch < -pitch_limit) pitch = -pitch_limit;

    // WASD + Q/E translation. Horizontal components follow the camera basis;
    // vertical (Q/E) is world-up so "down" always means toward the floor.
    // Polling the context directly keeps all integration inside update()
    // where deltaTime is known.
    if (deltaTime > 0.0f)
    {
        float fwd    = m_gameContext->get_action_value(ACTION_MOVE_FORWARD)
                     - m_gameContext->get_action_value(ACTION_MOVE_BACKWARD);
        float strafe = m_gameContext->get_action_value(ACTION_MOVE_RIGHT)
                     - m_gameContext->get_action_value(ACTION_MOVE_LEFT);
        float rise   = m_gameContext->get_action_value(ACTION_MOVE_UP)
                     - m_gameContext->get_action_value(ACTION_MOVE_DOWN);

        if (fwd != 0.0f || strafe != 0.0f || rise != 0.0f)
        {
            const float3 forward = get_forward();
            const float3 right   = get_right();
            const float3 up      = float3(0.0f, 1.0f, 0.0f);
            position += (forward * fwd + right * strafe + up * rise) * (move_speed * deltaTime);
        }
    }

    // Keep Primitive::rotation in sync for anyone reading it as a world
    // rotation (editor gizmos, serialization).
    rotation = Math::Quaternion<float>::rotation_from_axis_angle(float3(0.0f, 1.0f, 0.0f), yaw) *
               Math::Quaternion<float>::rotation_from_axis_angle(float3(1.0f, 0.0f, 0.0f), pitch);

    const float3 forward = get_forward();
    m_view_matrix = float4x4::look_at(position, position + forward, float3(0.0f, 1.0f, 0.0f));
}

void CameraComponent::mouse_wheel_move(float value)
{
    if (value != 0.0f)
        position += get_forward() * value;
}

CYBER_END_NAMESPACE
CYBER_END_NAMESPACE
