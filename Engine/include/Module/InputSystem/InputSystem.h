#pragma once
#include "Core/Core.h"
#include <gainput/gainput.h>

namespace Cyber
{
    enum Button
    {
        ButtonMenu,
        ButtonConfirm,
        MouseX,
        MouseY
    };

    class InputSystem
    {
    public:
        InputSystem();

        ~InputSystem();

        void initInputSystem(void* window);
        void updateInputSystem(void* window);
    private:
        Scope<gainput::InputManager> manager;
        Scope<gainput::InputMap> map;
        gainput::DeviceId mouseId;
        gainput::DeviceId keyboradId;
        gainput::DeviceId padId;
    };
}