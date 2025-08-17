#include "component/camera_component.h"
#include "application/application.h"

CYBER_BEGIN_NAMESPACE(Cyber)
CYBER_BEGIN_NAMESPACE(Component)

enum GameActions : Input::ActionID {
    ACTION_MOVE_FORWARD = 0,
    ACTION_MOVE_BACKWARD,
    ACTION_MOVE_LEFT,
    ACTION_MOVE_RIGHT,
    ACTION_JUMP,
    ACTION_FIRE,
    ACTION_AIM,
    ACTION_LOOK_X,
    ACTION_LOOK_Y,
    ACTION_PAUSE
};

CameraComponent::CameraComponent()
{
    auto inputMgr = Input::InputManager::get_instance();

    // Create game context
    m_gameContext = inputMgr->create_context("Game");
    
    auto* moveForward = m_gameContext->create_action(ACTION_MOVE_FORWARD, "MoveForward");
    auto* moveBackward = m_gameContext->create_action(ACTION_MOVE_BACKWARD, "MoveBackward");
    auto* moveLeft = m_gameContext->create_action(ACTION_MOVE_LEFT, "MoveLeft");
    auto* moveRight = m_gameContext->create_action(ACTION_MOVE_RIGHT, "MoveRight");

    // Bind keyboard controls
    m_gameContext->bind_action(ACTION_MOVE_FORWARD, Input::DeviceType::Keyboard, static_cast<Input::ButtonID>(Input::Key::W));
    m_gameContext->bind_action(ACTION_MOVE_BACKWARD, Input::DeviceType::Keyboard, static_cast<Input::ButtonID>(Input::Key::S));
    m_gameContext->bind_action(ACTION_MOVE_LEFT, Input::DeviceType::Keyboard, static_cast<Input::ButtonID>(Input::Key::A));
    m_gameContext->bind_action(ACTION_MOVE_RIGHT, Input::DeviceType::Keyboard, static_cast<Input::ButtonID>(Input::Key::D));

    // Set callbacks for actions
    moveForward->set_callback([](float value) {
        if (value > 0.0f) {
            cyber_log("Moving forward");
        }
    });
    
    // Push context to make it active
    inputMgr->push_context(m_gameContext);
}

void CameraComponent::update()
{

}

CYBER_END_NAMESPACE
CYBER_END_NAMESPACE