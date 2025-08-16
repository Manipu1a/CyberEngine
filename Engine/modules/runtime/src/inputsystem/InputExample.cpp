// Example usage of the new Input System in CyberEngine

#include "inputsystem/core/input_manager.h"
#include "inputsystem/core/input_context.h"
#include <iostream>

using namespace CyberEngine::Input;

// Define action IDs for your game
enum GameActions : ActionID {
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

class InputExample {
public:
    void Initialize() {
        // Get the input manager instance
        auto& inputMgr = InputManager::get_instance();
        
        // Initialize with window handle (pass actual HWND in real application)
        inputMgr.initialize(nullptr);

        // Create game context
        m_gameContext = inputMgr.create_context("Game");

        // Create actions
        auto* moveForward = m_gameContext->create_action(ACTION_MOVE_FORWARD, "MoveForward");
        auto* moveBackward = m_gameContext->create_action(ACTION_MOVE_BACKWARD, "MoveBackward");
        auto* moveLeft = m_gameContext->create_action(ACTION_MOVE_LEFT, "MoveLeft");
        auto* moveRight = m_gameContext->create_action(ACTION_MOVE_RIGHT, "MoveRight");
        auto* jump = m_gameContext->create_action(ACTION_JUMP, "Jump");
        auto* fire = m_gameContext->create_action(ACTION_FIRE, "Fire");
        auto* aim = m_gameContext->create_action(ACTION_AIM, "Aim");
        auto* pause = m_gameContext->create_action(ACTION_PAUSE, "Pause");

        // Bind keyboard controls
        m_gameContext->bind_action(ACTION_MOVE_FORWARD, DeviceType::Keyboard, static_cast<ButtonID>(Key::W));
        m_gameContext->bind_action(ACTION_MOVE_BACKWARD, DeviceType::Keyboard, static_cast<ButtonID>(Key::S));
        m_gameContext->bind_action(ACTION_MOVE_LEFT, DeviceType::Keyboard, static_cast<ButtonID>(Key::A));
        m_gameContext->bind_action(ACTION_MOVE_RIGHT, DeviceType::Keyboard, static_cast<ButtonID>(Key::D));
        m_gameContext->bind_action(ACTION_JUMP, DeviceType::Keyboard, static_cast<ButtonID>(Key::Space));
        m_gameContext->bind_action(ACTION_PAUSE, DeviceType::Keyboard, static_cast<ButtonID>(Key::Escape));
        
        // Bind mouse controls
        m_gameContext->bind_action(ACTION_FIRE, DeviceType::Mouse, static_cast<ButtonID>(MouseButton::Left));
        m_gameContext->bind_action(ACTION_AIM, DeviceType::Mouse, static_cast<ButtonID>(MouseButton::Right));

        // Bind gamepad controls (if available)
        m_gameContext->bind_action(ACTION_JUMP, DeviceType::Gamepad, static_cast<ButtonID>(GamepadButton::A));
        m_gameContext->bind_action(ACTION_FIRE, DeviceType::Gamepad, static_cast<ButtonID>(GamepadButton::RightTrigger));

        // Set callbacks for actions
        moveForward->set_callback([](float value) {
            if (value > 0.0f) {
                std::cout << "Moving forward\n";
            }
        });
        
        jump->set_callback([](float value) {
            if (value > 0.0f) {
                std::cout << "Jumping!\n";
            }
        });
        
        // Push context to make it active
        inputMgr.push_context(m_gameContext);

        // Create menu context (for UI)
        m_menuContext = inputMgr.create_context("Menu");

        // Menu controls might have different bindings
        auto* menuSelect = m_menuContext->create_action(0, "Select");
        auto* menuBack = m_menuContext->create_action(1, "Back");

        m_menuContext->bind_action(0, DeviceType::Keyboard, static_cast<ButtonID>(Key::Enter));
        m_menuContext->bind_action(1, DeviceType::Keyboard, static_cast<ButtonID>(Key::Escape));
        m_menuContext->bind_action(0, DeviceType::Mouse, static_cast<ButtonID>(MouseButton::Left));
    }
    
    void Update(float deltaTime) {
        auto& inputMgr = InputManager::get_instance();
        
        // Update input system
        inputMgr.update(deltaTime);

        // Check for input in game context
        if (m_gameContext->is_enabled()) {
            // Movement
            float moveX = 0.0f;
            float moveY = 0.0f;

            if (m_gameContext->is_action_pressed(ACTION_MOVE_FORWARD)) {
                moveY += 1.0f;
            }
            if (m_gameContext->is_action_pressed(ACTION_MOVE_BACKWARD)) {
                moveY -= 1.0f;
            }
            if (m_gameContext->is_action_pressed(ACTION_MOVE_LEFT)) {
                moveX -= 1.0f;
            }
            if (m_gameContext->is_action_pressed(ACTION_MOVE_RIGHT)) {
                moveX += 1.0f;
            }
            
            if (moveX != 0.0f || moveY != 0.0f) {
                // Apply movement
                MovePlayer(moveX, moveY, deltaTime);
            }
            
            // Check for jump (just pressed)
            if (m_gameContext->is_action_just_pressed(ACTION_JUMP)) {
                Jump();
            }
            
            // Check for fire (held)
            if (m_gameContext->is_action_pressed(ACTION_FIRE)) {
                Fire();
            }
            
            // Check for pause
            if (m_gameContext->is_action_just_pressed(ACTION_PAUSE)) {
                TogglePause();
            }
            
            // Mouse look
            float mouseX, mouseY;
            inputMgr.get_mouse_delta(mouseX, mouseY);
            if (mouseX != 0.0f || mouseY != 0.0f) {
                RotateCamera(mouseX, mouseY, deltaTime);
            }
        }
        
        // Quick access methods (bypass context)
        if (inputMgr.is_key_just_pressed(Key::F1)) {
            ShowHelp();
        }

        if (inputMgr.is_key_just_pressed(Key::F11)) {
            ToggleFullscreen();
        }
    }
    
    void SwitchToMenu() {
        auto& inputMgr = InputManager::get_instance();
        inputMgr.pop_context(); // Remove game context
        inputMgr.push_context(m_menuContext); // Add menu context
    }
    
    void SwitchToGame() {
        auto& inputMgr = InputManager::get_instance();
        inputMgr.pop_context(); // Remove menu context
        inputMgr.push_context(m_gameContext); // Add game context
    }
    
private:
    InputContext* m_gameContext = nullptr;
    InputContext* m_menuContext = nullptr;
    bool m_paused = false;
    
    void MovePlayer(float x, float y, float deltaTime) {
        // Implement player movement
        std::cout << "Moving: " << x << ", " << y << "\n";
    }
    
    void Jump() {
        std::cout << "Jump!\n";
    }
    
    void Fire() {
        std::cout << "Fire!\n";
    }
    
    void RotateCamera(float x, float y, float deltaTime) {
        // Implement camera rotation
    }
    
    void TogglePause() {
        m_paused = !m_paused;
        if (m_paused) {
            SwitchToMenu();
            std::cout << "Game Paused\n";
        } else {
            SwitchToGame();
            std::cout << "Game Resumed\n";
        }
    }
    
    void ShowHelp() {
        std::cout << "Help: WASD to move, Space to jump, Mouse to look\n";
    }
    
    void ToggleFullscreen() {
        std::cout << "Toggle Fullscreen\n";
    }
};

// Integration with CyberEngine Application
/*
In your SampleApp or main game class:

void SampleApp::initialize() {
    m_inputExample = std::make_unique<InputExample>();
    m_inputExample->Initialize();
}

void SampleApp::update(float deltaTime) {
    m_inputExample->Update(deltaTime);
    
    // Rest of your game update logic
}
*/