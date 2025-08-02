#include "component/camera_component.h"
#include "application/application.h"

CYBER_BEGIN_NAMESPACE(Cyber)
CYBER_BEGIN_NAMESPACE(Component)

CameraComponent::CameraComponent()
{
    input_manager = cyber_new<gainput::InputManager>();
    const gainput::DeviceId keyboardId = input_manager->CreateDevice<gainput::InputDeviceKeyboard>();
    const gainput::DeviceId mouseId = input_manager->CreateDevice<gainput::InputDeviceMouse>();

    input_manager->SetDisplaySize(1280, 720);

    map = cyber_new<gainput::InputMap>(*input_manager);
    map->MapBool(CameraMovement::Forward, keyboardId, gainput::KeyW);
    map->MapBool(CameraMovement::Backward, keyboardId, gainput::KeyS);
    map->MapBool(CameraMovement::Left, keyboardId, gainput::KeyA);
    map->MapBool(CameraMovement::Right, keyboardId, gainput::KeyD);
    map->MapBool(CameraMovement::Up, mouseId, gainput::MouseButtonLeft);
}

void CameraComponent::update()
{
    input_manager->Update();

    if(map->GetBoolWasDown(CameraMovement::Forward))
    {
        
    }
}

CYBER_END_NAMESPACE
CYBER_END_NAMESPACE