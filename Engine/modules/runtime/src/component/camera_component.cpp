#include "component/camera_component.h"
#include "application/application.h"

CYBER_BEGIN_NAMESPACE(Cyber)
CYBER_BEGIN_NAMESPACE(Component)

enum GameActions : Input::ActionID {
    ACTION_MOVE_FORWARD = 0,
    ACTION_MOVE_BACKWARD,
    ACTION_MOVE_LEFT,
    ACTION_MOVE_RIGHT,
    ACTION_MOUSE_LEFT,
    ACTION_LOOK_X,
    ACTION_LOOK_Y,
    ACTION_PAUSE
};

CameraComponent::CameraComponent(float3 _camera_position) : camera_position(_camera_position)
{
    // Initialize the camera component
    yaw = 0.0f;
    pitch = 0.0f;
    roll = 0.0f;
    
    // Create input context for game controls
    auto inputMgr = Input::InputManager::get_instance();
    
    // Create game context
    m_gameContext = inputMgr->create_context("Game");
    
    auto* moveForward = m_gameContext->create_action(ACTION_MOVE_FORWARD, "MoveForward");
    auto* moveBackward = m_gameContext->create_action(ACTION_MOVE_BACKWARD, "MoveBackward");
    auto* moveLeft = m_gameContext->create_action(ACTION_MOVE_LEFT, "MoveLeft");
    auto* moveRight = m_gameContext->create_action(ACTION_MOVE_RIGHT, "MoveRight");
    auto* mouse_left = m_gameContext->create_action(ACTION_MOUSE_LEFT, "MouseLeft");

    // Bind keyboard controls
    m_gameContext->bind_action(ACTION_MOVE_FORWARD, Input::DeviceType::Keyboard, static_cast<Input::ButtonID>(Input::Key::W));
    m_gameContext->bind_action(ACTION_MOVE_BACKWARD, Input::DeviceType::Keyboard, static_cast<Input::ButtonID>(Input::Key::S));
    m_gameContext->bind_action(ACTION_MOVE_LEFT, Input::DeviceType::Keyboard, static_cast<Input::ButtonID>(Input::Key::A));
    m_gameContext->bind_action(ACTION_MOVE_RIGHT, Input::DeviceType::Keyboard, static_cast<Input::ButtonID>(Input::Key::D));
    m_gameContext->bind_action(ACTION_MOUSE_LEFT, Input::DeviceType::Mouse, static_cast<Input::ButtonID>(Input::MouseButton::Left));

    // Set callbacks for actions
    moveForward->set_callback([this](float value){
        move_forward(value);
    });
    moveBackward->set_callback([this](float value){
        move_backward(value);
    });
    moveLeft->set_callback([this](float value){
        move_left(value);
    });
    moveRight->set_callback([this](float value){
        move_right(value);
    });

    // Push context to make it active
    inputMgr->push_context(m_gameContext);
}

void CameraComponent::update()
{
    auto inputMgr = Input::InputManager::get_instance();

    float delta_mouse_x = 0.0f;
    float delta_mouse_y = 0.0f;
    float x, y;
    inputMgr->get_mouse_position(x, y);
    if(m_gameContext->is_action_pressed(ACTION_MOUSE_LEFT))
    {
        delta_mouse_x = x - last_mouse_x;
        delta_mouse_y = y - last_mouse_y;

        yaw += delta_mouse_x * 0.1f;
        pitch += delta_mouse_y * 0.1f;
    }
    last_mouse_x = x;
    last_mouse_y = y;

    rotation = Math::Quaternion<float>::rotation_from_axis_angle(float3(1.0f, 0.0f, 0.0f), -pitch) * 
    Math::Quaternion<float>::rotation_from_axis_angle(float3(0.0f, 1.0f, 0.0f), -yaw);
}

void CameraComponent::move_forward(float value)
{
    if(value > 0.0f)
    {
        camera_position += float3(0.0f, 0.0f, 1.0f) * value;
    }
}

void CameraComponent::move_backward(float value)
{
    if(value > 0.0f)
    {
        camera_position += float3(0.0f, 0.0f, -1.0f) * value;
    }
}

void CameraComponent::move_left(float value)
{
    if(value > 0.0f)
    {
        camera_position += float3(-1.0f, 0.0f, 0.0f) * value;
    }
}

void CameraComponent::move_right(float value)
{
    if(value > 0.0f)
    {
        camera_position += float3(1.0f, 0.0f, 0.0f) * value;
    }
}

CYBER_END_NAMESPACE
CYBER_END_NAMESPACE