#include "InputSystem/InputSystem.h"
#include <Module/Log/Log.h>
#include <glfw/glfw3.h>
#include <glfw/glfw3native.h>
#include "InputSystem/InputListener.h"

namespace Cyber
{
    InputSystem::InputSystem()
    {

    }   

    InputSystem::~InputSystem()
    {
        
    }

    void InputSystem::initInputSystem()
    {
        manager = CreateScope<gainput::InputManager>();
        mouseId = manager->CreateDevice<gainput::InputDeviceMouse>();
        keyboradId = manager->CreateDevice<gainput::InputDeviceKeyboard>();
        padId = manager->CreateDevice<gainput::InputDevicePad>();
        map = CreateScope<gainput::InputMap>(*manager.get());
        map->MapBool(Button::ButtonMenu, keyboradId, gainput::KeyQ);
        map->MapBool(Button::ButtonConfirm, mouseId, gainput::MouseButtonLeft);
        map->MapFloat(Button::MouseX, mouseId, gainput::MouseAxisX);
        map->MapFloat(Button::MouseY, mouseId, gainput::MouseAxisY);
        map->MapFloat(Button::ButtonConfirm, mouseId, gainput::PadButtonA);

        //DeviceInputListener buttonListener(*manager.get(), 1);
        //manager->AddListener(&buttonListener);
        manager->SetDisplaySize(1280,720);
    }

    void InputSystem::updateInputSystem(void* window)
    {
        manager->Update();
        MSG msg;
        HWND hWnd = (HWND)window;
        
		while (PeekMessage(&msg, hWnd,  0, 0, PM_REMOVE)) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
\
			// Forward any input messages to Gainput
			manager->HandleMessage(msg);
		}
        
        if(map->GetFloatDelta(Button::MouseX) != 0.0f || map->GetFloatDelta(Button::MouseY) != 0.0f)
        {
            //CB_CORE_INFO("Mouse: {0}, {1}", map->GetFloat(Button::MouseX), map->GetFloat(Button::MouseY));
        }

        if(map->GetBoolWasDown(ButtonMenu))
        {
            CB_CORE_INFO("ButtonMenu");
        }

    }
}