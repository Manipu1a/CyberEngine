#include "inputsystem/InputSystem.h"
#include "CyberLog/Log.h"
#include "inputsystem/InputListener.h"

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
        /*
        manager = CreateScope<gainput::InputManager>();
        mouseId = manager->CreateDevice<gainput::InputDeviceMouse>();
        padId = manager->CreateDevice<gainput::InputDevicePad>();
        keyboradId = manager->CreateDevice<gainput::InputDeviceKeyboard>();
        map = CreateScope<gainput::InputMap>(*manager.get());
        map->MapBool(Button::ButtonMenu, keyboradId, gainput::KeyQ);
        map->MapBool(Button::ButtonConfirm, mouseId, gainput::MouseButtonLeft);
        map->MapFloat(Button::MouseX, mouseId, gainput::MouseAxisX);
        map->MapFloat(Button::MouseY, mouseId, gainput::MouseAxisY);
        map->MapFloat(Button::ButtonConfirm, mouseId, gainput::PadButtonA);

        //DeviceInputListener buttonListener(*manager.get(), 1);
        //manager->AddListener(&buttonListener);
        manager->SetDisplaySize(1280,720);
        */

        mInputCaptured = false;
        mDefaultCapture = true;
        pDeviceTypes.resize(MAX_INPUT_GAMEPADS + 4);
        mControls.resize(MAX_DEVICES);

        // Create input manager
        pInputManager = CreateScope<gainput::InputManager>();
        pInputManager->SetDisplaySize(1280,720);
        // Create all necessary devices
        mMouseDeviceID = pInputManager->CreateDevice<gainput::InputDeviceMouse>();
        mKeyboardDeviceID = pInputManager->CreateDevice<gainput::InputDeviceKeyboard>();
        // Assign device types
        pDeviceTypes[mMouseDeviceID] = InputDeviceType::INPUT_DEVICE_MOUSE;
        pDeviceTypes[mKeyboardDeviceID] = InputDeviceType::INPUT_DEVICE_KEYBOARD;

        mControls[mKeyboardDeviceID].resize(gainput::KeyCount_);
        mControls[mMouseDeviceID].resize(gainput::MouseButtonCount_);

        mInputActionMappingIdToDesc.resize(MAX_DEVICES);
        mInputActionMappingIdToDesc[mMouseDeviceID].resize(MAX_INPUT_ACTIONS);
        mInputActionMappingIdToDesc[mKeyboardDeviceID].resize(MAX_INPUT_ACTIONS);

        pInputManager->AddListener(this);

        //Global text inputaction
        mGlobalTextInputControl.pUserData = this;
        mGlobalTextInputControl.pFunction = [](InputActionContext* ctx)
        {
            CB_CORE_INFO("Text Input:{0}", ctx->mActionId);
            return true;
        };

        for(uint32_t i = 0u; i < (uint32_t)KeyboardMouseInputActions::DUMP_PROFILE_DATA; ++i)
        {
            InputActionDesc actionDesc = {i, KeyChecker};
            addInputAction(actionDesc);
        }

        gainput::InputDeviceKeyboard* keyboard = (gainput::InputDeviceKeyboard*)pInputManager->GetDevice(mKeyboardDeviceID);
        keyboard->SetTextInputEnabled(true);
    }

    void InputSystem::updateInputSystem(void* window)
    {
        pInputManager->Update();
        MSG msg;
        HWND hWnd = (HWND)window;
        
        gainput::InputDeviceKeyboard* keyboard = (gainput::InputDeviceKeyboard*)pInputManager->GetDevice(mKeyboardDeviceID);

        if(keyboard)
        {
            uint32_t count = 0;
            char pText = keyboard->GetNextCharacter();
            if(pText)
            {
                //CB_CORE_INFO("Text Input:{0}", pText);
            }
        }

		while (PeekMessage(&msg, hWnd,  0, 0, PM_REMOVE)) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			// Forward any input messages to Gainput
			pInputManager->HandleMessage(msg);
		}

    }
    void InputSystem::addDefaultActionMappings()
    {
       
        eastl::vector<ActionMappingDesc> actionMappingsArr = {
            
        };

        AddActionMappings(actionMappingsArr, actionMappingsArr.size(), InputActionMappingDeviceTarget::INPUT_ACTION_MAPPING_TARGET_ALL);
    }

    void InputSystem::addInputAction(const InputActionDesc& pDesc, const InputActionMappingDeviceTarget actionMappingTarget)
    {
        cyber_assert(pDesc.mActionId < MAX_INPUT_ACTIONS, "Input action greater than MAX!");
        AddInputAction(pDesc, actionMappingTarget);
    }

    void InputSystem::AddActionMappings(const eastl::vector<ActionMappingDesc> actionMappings, const uint32_t numActions, const InputActionMappingDeviceTarget actionMappingTarget)
    {
        // Ensure there isn't too many actions than we can fit in memory
        cyber_assert(numActions < MAX_INPUT_ACTIONS, "Too many actions!");

        // First need to reset mappings
        mButtonControlPerformQueue.clear();
        mFloatDeltaControlCancelQueue.clear();

        for(uint32_t i = 0;i < numActions;++i)
        {
            const ActionMappingDesc& pActionMappingDesc = actionMappings[i];
            cyber_check(pActionMappingDesc.mActionMappingDeviceTarget != InputActionMappingDeviceTarget::INPUT_ACTION_MAPPING_TARGET_ALL);

            cyber_assert(pActionMappingDesc.mActionId < MAX_INPUT_ACTIONS, "mActionId is greater than MAX_INPUT_ACTIONS");

            gainput::DeviceId deviceId = ~0u;

            switch (pActionMappingDesc.mActionMappingDeviceTarget)
            {
            case InputActionMappingDeviceTarget::INPUT_ACTION_MAPPING_TARGET_CONTROLLER:
            {
                deviceId = pGamepadDeviceIDs[pActionMappingDesc.mUserId];
                break;
            }
            case InputActionMappingDeviceTarget::INPUT_ACTION_MAPPING_TARGET_KEYBOARD:
            {
                deviceId = mKeyboardDeviceID;
                break;
            }
            case InputActionMappingDeviceTarget::INPUT_ACTION_MAPPING_TARGET_MOUSE:
            {
                deviceId = mMouseDeviceID;
                break;
            }
            case InputActionMappingDeviceTarget::INPUT_ACTION_MAPPING_TARGET_TOUCH:
            {
                deviceId = mTouchDeviceID;
                break;
            }
            default:
            {
                cyber_assert(0, "Can't match device type!");
            }
            }

            cyber_check(deviceId != ~0u);

            switch (pActionMappingDesc.mActionMappingType)
            {
            case InputActionMappingType::INPUT_ACTION_MAPPING_NORMAL:
            case InputActionMappingType::INPUT_ACTION_MAPPING_COMPOSITE:
            case InputActionMappingType::INPUT_ACTION_MAPPING_COMBO:
            case InputActionMappingType::INPUT_ACTION_MAPPING_TOUCH_GESTURE:
            case InputActionMappingType::INPUT_ACTION_MAPPING_TOUCH_VIRTUAL_JOYSTICK:
            {
                cyber_check(mInputActionMappingIdToDesc[deviceId][pActionMappingDesc.mActionId].mActionMappingDeviceTarget == InputActionMappingDeviceTarget::INPUT_ACTION_MAPPING_TARGET_ALL);
                mInputActionMappingIdToDesc[deviceId][pActionMappingDesc.mActionId] = pActionMappingDesc;
                // Register an action for UI action mappings so that the app can intercept them via the global action (GLOBAL_INPUT_ACTION_ANY_BUTTON_ACTION)
                if(pActionMappingDesc.mActionId > 0)
                {
                    // Ensure the type is INPUT_ACTION_MAPPING_NORMAL
                    cyber_assert(pActionMappingDesc.mActionMappingType == InputActionMappingType::INPUT_ACTION_MAPPING_NORMAL, "Action mapping type isn't INPUT_ACTION_MAPPING_NORMAL");

                    InputActionDesc actionDesc;
                    actionDesc.mActionId = pActionMappingDesc.mActionId;
                    actionDesc.mUserId = pActionMappingDesc.mUserId;
                    AddInputAction(actionDesc, pActionMappingDesc.mActionMappingDeviceTarget);
                }
            }
            default:
            {
                cyber_assert(0,"Can't match action mapping type!");
            }
            }


        }
    }

    void InputSystem::AddInputAction(const InputActionDesc& pDesc, const InputActionMappingDeviceTarget actionMappingTarget)
    {
        cyber_assert(pDesc.mActionId < MAX_INPUT_ACTIONS, "Input action greater than MAX!");

        //keyboard action
        if(actionMappingTarget == InputActionMappingDeviceTarget::INPUT_ACTION_MAPPING_TARGET_KEYBOARD || actionMappingTarget == InputActionMappingDeviceTarget::INPUT_ACTION_MAPPING_TARGET_ALL)
        {
            const ActionMappingDesc& pActionMappingDesc = mInputActionMappingIdToDesc[mKeyboardDeviceID][pDesc.mActionId];
            if(actionMappingTarget == InputActionMappingDeviceTarget::INPUT_ACTION_MAPPING_TARGET_KEYBOARD)
            {
                cyber_assert(pActionMappingDesc.mActionMappingDeviceTarget != InputActionMappingDeviceTarget::INPUT_ACTION_MAPPING_TARGET_ALL, "mActionMappingDeviceTarget is INPUT_ACTION_MAPPING_TARGET_ALL");
            }

            if(pActionMappingDesc.mActionMappingDeviceTarget != InputActionMappingDeviceTarget::INPUT_ACTION_MAPPING_TARGET_ALL)
                CreateActionForActionMapping(pActionMappingDesc, pDesc);
        }

        //mouse action
        if(actionMappingTarget == InputActionMappingDeviceTarget::INPUT_ACTION_MAPPING_TARGET_MOUSE || actionMappingTarget == InputActionMappingDeviceTarget::INPUT_ACTION_MAPPING_TARGET_ALL)
        {
            const ActionMappingDesc& pActionMappingDesc = mInputActionMappingIdToDesc[mMouseDeviceID][pDesc.mActionId];
            if(actionMappingTarget == InputActionMappingDeviceTarget::INPUT_ACTION_MAPPING_TARGET_MOUSE)
            {
                cyber_assert(pActionMappingDesc.mActionMappingDeviceTarget != InputActionMappingDeviceTarget::INPUT_ACTION_MAPPING_TARGET_ALL, "mActionMappingDeviceTarget is INPUT_ACTION_MAPPING_TARGET_ALL");
            }

            if(pActionMappingDesc.mActionMappingDeviceTarget != InputActionMappingDeviceTarget::INPUT_ACTION_MAPPING_TARGET_ALL)
                CreateActionForActionMapping(pActionMappingDesc, pDesc);
        }
    }

    void InputSystem::CreateActionForActionMapping(const ActionMappingDesc& pActionMappingDesc, const InputActionDesc& pActionDesc)
    {
        switch (pActionMappingDesc.mActionMappingDeviceTarget)
        {
            case InputActionMappingDeviceTarget::INPUT_ACTION_MAPPING_TARGET_KEYBOARD:
            {
                switch (pActionMappingDesc.mActionMappingType) 
                {
                    case InputActionMappingType::INPUT_ACTION_MAPPING_NORMAL:
                    {
                        IControl* pControl = AllocateControl<IControl>(mKeyboardDeviceID);
                        cyber_assert(pControl, "Allocate failed!");

                        pControl->mType = InputControlType::CONTROL_BUTTON;
                        pControl->mAction = pActionDesc;
                        mControls[mKeyboardDeviceID][pActionMappingDesc.mDeviceButtons[0]].push_back(*pControl);
                        break;
                    }
                    case InputActionMappingType::INPUT_ACTION_MAPPING_COMPOSITE:
                    {
                        CompositeControl* pControl = AllocateControl<CompositeControl>(mKeyboardDeviceID);
                        cyber_assert(pControl, "Allocate failed!");

                        memset((void*)pControl, 0, sizeof(*pControl));
                        pControl->mComposite = 4;
                        pControl->mControls[0] = pActionMappingDesc.mDeviceButtons[0];
                        pControl->mControls[1] = pActionMappingDesc.mDeviceButtons[1];
                        pControl->mControls[2] = pActionMappingDesc.mDeviceButtons[2];
                        pControl->mControls[3] = pActionMappingDesc.mDeviceButtons[3];
                        pControl->mType = InputControlType::CONTROL_COMPOSITE;
                        pControl->mAction = pActionDesc;
                        for(uint32_t i = 0;i < pControl->mComposite;++i)
                        {
                            mControls[mKeyboardDeviceID][pControl->mControls[i]].push_back(*pControl);
                        }
                        break;
                    }
                    case InputActionMappingType::INPUT_ACTION_MAPPING_COMBO:
                    {
                        ComboControl* pControl = AllocateControl<ComboControl>(mKeyboardDeviceID);
                        cyber_assert(pControl, "Allocate failed!");

                        pControl->mType = InputControlType::COMTROL_COMBO;
                        pControl->mAction = pActionDesc;
                        pControl->mPressButton = pActionMappingDesc.mDeviceButtons[0];
                        pControl->mTriggerButton = pActionMappingDesc.mDeviceButtons[1];
                        mControls[mKeyboardDeviceID][pActionMappingDesc.mDeviceButtons[0]].push_back(*pControl);
                        mControls[mKeyboardDeviceID][pActionMappingDesc.mDeviceButtons[1]].push_back(*pControl);
                        break;
                    }
                    default:
                        cyber_assert(false, "Can't match InputActionMappingType!");
                }
                break;
            }
        }
    }

    bool InputSystem::OnDeviceButtonBool(gainput::DeviceId deviceId, gainput::DeviceButtonId deviceButton, bool oldValue, bool newValue)
    {
        if(oldValue == newValue)
            return false;

        const gainput::InputDevice* device = pInputManager->GetDevice(deviceId);
		char buttonName[64] = "";
		device->GetButtonName(deviceButton, buttonName, 64);
        CB_CORE_INFO("Device:{0} button:{1}", deviceId, buttonName);

        if(mControls[deviceId].size() > 0)
        {
            InputActionContext ctx = {};
            ctx.mDeviceType = (uint8_t)pDeviceTypes[deviceId];
            ctx.pCaptured = IsPointerType(deviceId) ? mInputCaptured : mDefaultCapture;

            if(IsPointerType(deviceId))
            {
                gainput::InputDeviceMouse* pMouse = (gainput::InputDeviceMouse*)pInputManager->GetDevice(mMouseDeviceID);
                mMousePosition[0] = pMouse->GetFloat(gainput::MouseAxisX);
                mMousePosition[1] = pMouse->GetFloat(gainput::MouseAxisY);
                ctx.pPosition = &mMousePosition;
                ctx.mScrollValue = pMouse->GetFloat(gainput::MouseButtonMiddle);                
            }

            bool executeNext = true;

            for(uint32_t i = 0;i < mControls[deviceId][deviceButton].size(); ++i)
            {
                IControl& control = mControls[deviceId][deviceButton][i];
                if(!executeNext)
                    return true;

                const InputControlType type = control.mType;
                const InputActionDesc& pDesc = control.mAction;
                InputActionContext ctx = {};
                ctx.pUserData = pDesc.pUserData;
                ctx.mActionId = pDesc.mActionId;

                switch (type)
                {
                case InputControlType::CONTROL_BUTTON:
                {
                    ctx.mBool = newValue;
                    if(newValue && !oldValue)
                    {
                        ctx.mPhase = (uint8_t)InputActionPhase::INPUT_ACTION_PHASE_STARTED;
                        if(pDesc.pFunction)
                            executeNext = pDesc.pFunction(&ctx) && executeNext;
                        if(mGlobalAnyButtonAction.pFunction)
                            mGlobalAnyButtonAction.pFunction(&ctx);

                        ctx.mPhase = (uint8_t)InputActionPhase::INPUT_ACTION_PHASE_UPDATED;
                        if(pDesc.pFunction)
                            executeNext = pDesc.pFunction(&ctx) && executeNext;
                        if(mGlobalAnyButtonAction.pFunction)
                            mGlobalAnyButtonAction.pFunction(&ctx);
                    }
                    else if(oldValue && !newValue)
                    {
                        ctx.mPhase = (uint8_t)InputActionPhase::INPUT_ACTION_PHASE_CANCELED;
                        if(pDesc.pFunction)
                            executeNext = pDesc.pFunction(&ctx) && executeNext;
                        if(mGlobalAnyButtonAction.pFunction)
                            mGlobalAnyButtonAction.pFunction(&ctx);
                    }
                    break;
                }
                case InputControlType::CONTROL_COMPOSITE:
                {
                    CompositeControl* pControl = (CompositeControl*)(&control);
                    uint32_t index = 0;
                    for(; index < pControl->mComposite; ++index)
                        if(deviceButton == pControl->mControls[index])
                            break;

                    const uint32_t axis = (index > 1) ? 1 : 0;
                    if(newValue)
                    {
                        pControl->mPressedVal[index] = 1;
                        pControl->mValue[axis] = (float)pControl->mPressedVal[axis * 2 + 0];
                    }

                    if(pControl->mComposite == 2)
                    {
                        ctx.mFloat = pControl->mValue[axis];
                    }
                    else 
                    {
                        if(!pControl->mValue[0] && !pControl->mValue[1])
                            ctx.mFloat2 = DirectX::XMFLOAT2(0.0f, 0.0f);
                        else
                            ctx.mFloat2 = pControl->mValue;
                    }

                    // Action Started
                    if(!pControl->mStarted && !oldValue && newValue)
                    {
                        pControl->mStarted = 1;
                        ctx.mPhase = (uint8_t)InputActionPhase::INPUT_ACTION_PHASE_STARTED;
                        if(pDesc.pFunction)
                            executeNext = pDesc.pFunction(&ctx) && executeNext;
                    }
                    // Action Performed
                    if(pControl->mStarted && newValue && !pControl->mPerformed[index])
                    {
                        pControl->mPerformed[index] = 1;
                        ctx.mPhase = (uint8_t)InputActionPhase::INPUT_ACTION_PHASE_UPDATED;
                        if(pDesc.pFunction)
                            executeNext = pDesc.pFunction(&ctx) && executeNext;
                    }
                    // Action Canceled
                    if(oldValue && !newValue)
                    {
                        pControl->mPerformed[index] = 0;
                        pControl->mPressedVal[index] = 0;
                        bool allReleased = true;
                        for(uint8_t i = 0;i < pControl->mComposite;++i)
                        {
                            if(pControl->mPerformed[i])
                            {
                                allReleased = false;
                                break;
                            }
                        }

                        if(allReleased)
                        {
                            pControl->mValue = DirectX::XMFLOAT2(0.0f, 0.0f);
                            pControl->mStarted = 0;
                            ctx.mFloat2 = pControl->mValue;
                            ctx.mPhase = (uint8_t)InputActionPhase::INPUT_ACTION_PHASE_CANCELED;
                            if(pDesc.pFunction)
                                executeNext = pDesc.pFunction(&ctx) && executeNext;
                        }
                        else if(pDesc.pFunction)
                        {
                            ctx.mPhase = (uint8_t)InputActionPhase::INPUT_ACTION_PHASE_UPDATED;
                            pControl->mValue[axis] = pControl->mPressedVal[axis * 2 + 0] - pControl->mPressedVal[axis * 2 + 1];
                            ctx.mFloat2 = pControl->mValue;
                            executeNext = pDesc.pFunction(&ctx) && executeNext;
                        }
                    }
                    break;
                }
                case InputControlType::CONTROL_FLOAT:
                {
                    if(!oldValue && newValue)
                    {
                        cyber_check(deviceButton == gainput::MouseButtonWheelUp || deviceButton == gainput::MouseButtonWheelDown);

                        FloatControl* pControl = (FloatControl*)(&control);
                        ctx.mFloat2[1] = deviceButton == gainput::MouseButtonWheelUp ? 1.0f : -1.0f;
                        if(pDesc.pFunction)
                        {
                            ctx.mPhase = (uint8_t)InputActionPhase::INPUT_ACTION_PHASE_UPDATED;
                            executeNext = pDesc.pFunction(&ctx) && executeNext;
                        }

                        FloatControlSet val = {pControl};
                        mFloatDeltaControlCancelQueue.push_back(val);
                    }
                    break;
                }
                case InputControlType::COMTROL_COMBO:
                {
                    ComboControl* pControl = (ComboControl*)(&control);
                    if(deviceButton == pControl->mPressButton)
                    {
                        pControl->mPressed = newValue;
                    }
                    else if(pControl->mPressed && oldValue && !newValue && pDesc.pFunction)
                    {
                        ctx.mBool = true;
                        ctx.mPhase = (uint8_t)InputActionPhase::INPUT_ACTION_PHASE_UPDATED;
                        pDesc.pFunction(&ctx);
                    }
                    break;
                }
                default:
                    break;
                }
            }
        }
		return false;
    }

    bool InputSystem::IsPointerType(gainput::DeviceId device) const
    {
        return (device == mMouseDeviceID);
    }

    bool InputSystem::KeyChecker(InputActionContext* ctx)
    {
        const uint32_t actionId = ctx->mActionId;

        CB_CORE_INFO("Inputaction{0}", actionId);

        return true;
    }
}