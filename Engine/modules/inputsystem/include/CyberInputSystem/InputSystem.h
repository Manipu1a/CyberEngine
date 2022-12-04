#pragma once
#include "cyber_inputsystem.config.h"
#include "core/Core.h"
#include <gainput/gainput.h>
#include <EASTL/vector.h>
#include <EASTL/map.h>
#include <DirectXMath.h>
#include <mimalloc.h>

namespace Cyber
{
    enum class KeyboardMouseInputActions : uint32_t
    {
        // Keyboard
        ESCAPE = 0,
        F1,
		F2,
		F3,
		F4,
		F5,
		F6,
		F7,
		F8,
		F9,
		F10,
		F11,
		F12,
		BREAK,
		INSERT,
		DEL,
		NUM_LOCK,
		KP_MULTIPLY,

		ACUTE,
		NUM_1,
		NUM_2,
		NUM_3,
		NUM_4,
		NUM_5,
		NUM_6,
		NUM_7, 
		NUM_8,
		NUM_9,
		NUM_0,
		MINUS,
		EQUAL,
		BACK_SPACE,
		KP_ADD,
		KP_SUBTRACT,

		TAB,
		Q,
		W,
		E,
		R,
		T,
		Y,
		U,
		I,
		O,
		P,
		BRACKET_LEFT,
		BRACKET_RIGHT,
		BACK_SLASH,
		KP_7_HOME,
		KP_8_UP,
		KP_9_PAGE_UP,

		CAPS_LOCK,
		A,
		S,
		D,
		F,
		G,
		H,
		J,
		K,
		L, 
		SEMICOLON,
		APOSTROPHE,
		ENTER,
		KP_4_LEFT, 
		KP_5_BEGIN, 
		KP_6_RIGHT,

		SHIFT_L,
		Z,
		X,
		C,
		V,
		B,
		N,
		M,
		COMMA,
		PERIOD,
		FWRD_SLASH,
		SHIFT_R,
		KP_1_END, 
		KP_2_DOWN, 
		KP_3_PAGE_DOWN, 

		CTRL_L,
		ALT_L,
		SPACE,
		ALT_R,
		CTRL_R,
		LEFT,
		RIGHT,
		UP,
		DOWN,
		KP_0_INSERT,

        // Mouse
		LEFT_CLICK,
		MID_CLICK,
		RIGHT_CLICK,
		SCROLL_UP,
		SCROLL_DOWN,

		// Full screen / dump data actions (we always need those)
		DUMP_PROFILE_DATA,
		TOGGLE_FULLSCREEN,
    };

    enum Button
    {
        ButtonMenu,
        ButtonConfirm,
        MouseX,
        MouseY
    };

    enum class InputControlType
    {
        CONTROL_BUTTON = 0,
        CONTROL_FLOAT,
        CONTROL_AXIS,
        CONTROL_VIRTUAL_JOYSTICK,
        CONTROL_COMPOSITE,
        COMTROL_COMBO
    };

    enum class InputDeviceType
    {
        INPUT_DEVICE_INVALID = 0,
        INPUT_DEVICE_GAMEPAD,
        INPUT_DEVICE_TOUCH,
        INPUT_DEVICE_KEYBOARD,
        INPUT_DEVICE_MOUSE,
    };
    enum class InputActionPhase
    {
        /// Action is initiated
        INPUT_ACTION_PHASE_STARTED = 0,
        /// Example: mouse delta changed, key pressed, ...
        INPUT_ACTION_PHASE_UPDATED,
        /// Example: mouse delta changed, key pressed, ...
        INPUT_ACTION_PHASE_ENDED,
        /// Example: left mouse button was pressed and now released, gesture was started but got canceled
        INPUT_ACTION_PHASE_CANCELED,
    };

    struct InputActionContext
    {
        void* pUserData = nullptr;

        union
        {
            /// Gesture input
            DirectX::XMFLOAT4 mFloat4;
            /// 3D input (gyroscope, ...)
            DirectX::XMFLOAT3 mFloat3;
            /// 2D input (mouse position, delta, composite input (wasd), gamepad stick, joystick, ...)
            DirectX::XMFLOAT2 mFloat2;
            /// 1D input (composite input (ws), gamepad left trigger, ...)
            float mFloat;
            /// Button input (mouse left button, keyboard keys, ...)
            bool mBool;
            /// Text input
            wchar_t* pText;
        };
        DirectX::XMFLOAT2* pPosition = nullptr;
        bool pCaptured = false;
        float mScrollValue = 0.f;
        uint32_t mActionId = UINT_MAX;
        uint8_t mPhase = (uint8_t)InputActionPhase::INPUT_ACTION_PHASE_ENDED;
        uint8_t mDeviceType = (uint8_t)InputDeviceType::INPUT_DEVICE_INVALID;
    };

    using InputActionCallBack = bool (*)(InputActionContext* pContext);
    //typedef bool (*InputActionCallback)(InputActionContext* pContext);
    struct InputActionDesc
    {
        /// Action ID 
        uint32_t mActionId = UINT_MAX;
        /// Callback when an action is initiated, performed or canceled
        InputActionCallBack pFunction = nullptr;
        /// User data which will be assigned to InputActionContext::pUserData when calling pFunction
        void* pUserData = nullptr;
        /// User management (which user does this action apply to)
        uint8_t mUserId = 0u;

        bool operator==(InputActionDesc const& rhs) const
        {
            return (this->mActionId == rhs.mActionId && this->mUserId == rhs.mUserId);
        }
    };

    // Global actions
    // These are handled differently than actions from an action mapping.
    struct GlobalInputActionDesc
    {
        enum class GlobalInputActionType
        {
            // Triggered when any action mapping of a button is triggered
            ANY_BUTTON_ACTION = 0,
            // Used for processing text from keyboard or virtual keyboard
            TEXT
        };

        GlobalInputActionType mGlobalInputActionType = GlobalInputActionType::ANY_BUTTON_ACTION;

        /// Callback when an action is initiated, performed or canceled
        InputActionCallBack pFunction = nullptr;
        /// User data which will be assigned to InputActionContext::pUserData when calling pFunction
        void* pUserData = nullptr;
    };

    struct IControl
    {
        InputActionDesc mAction;
        InputControlType mType;
    };
    
    struct CompositeControl : public IControl
    {
        CompositeControl(const uint32_t controls[4], uint8_t composite)
        {
			memset((void*)this, 0, sizeof(*this));
            mComposite = composite;
            memcpy(mControls, controls, sizeof(mControls));
            mType = InputControlType::CONTROL_COMPOSITE;
        }

        DirectX::XMFLOAT2 mValue;
        uint32_t mControls[4];
        uint8_t mComposite;
        uint8_t mStarted;
        uint8_t mPerformed[4];
        uint8_t mPressedVal[4];
    };

    struct FloatControl: public IControl
    {
        FloatControl(uint16_t start, uint8_t target, bool raw, bool delta)
        {
            memset((void*)this, 0, sizeof(*this));
            mStartButton = start;
            mTarget = target;
            mType = InputControlType::CONTROL_FLOAT;
            mDelta = (1 << (uint8_t)raw) | (uint8_t)delta;
            mScale = 1;
            mScaleByDT = false;
        }

        DirectX::XMFLOAT3 mValue;
        float mScale;
        uint16_t mStartButton;
        uint8_t mTarget;
        uint8_t mStarted;
        uint8_t mPerformed;
        uint8_t mDelta;
        bool mScaleByDT;
    };

    struct FloatControlSet
    {
        FloatControl* key;
    };

    struct IControlSet
    {
        IControl* key;
    };

    struct ComboControl : public IControl
    {
        uint16_t mPressButton;
        uint16_t mTriggerButton;
        uint8_t mPressed;
    };

    enum class InputActionMappingType
    {
        INPUT_ACTION_MAPPING_NORMAL = 0,

        INPUT_ACTION_MAPPING_COMPOSITE,

        INPUT_ACTION_MAPPING_COMBO,

        INPUT_ACTION_MAPPING_TOUCH_GESTURE,

        INPUT_ACTION_MAPPING_TOUCH_VIRTUAL_JOYSTICK
    };

    enum class InputActionMappingDeviceTarget
    {
        INPUT_ACTION_MAPPING_TARGET_ALL = 0,
        INPUT_ACTION_MAPPING_TARGET_CONTROLLER,
        INPUT_ACTION_MAPPING_TARGET_KEYBOARD,
        INPUT_ACTION_MAPPING_TARGET_MOUSE,
        INPUT_ACTION_MAPPING_TARGET_TOUCH,
    };

    struct ActionMappingDesc
    {
        // The type of the action mapping
        InputActionMappingType mActionMappingType = InputActionMappingType::INPUT_ACTION_MAPPING_NORMAL;
        // Type of device this action targets
        InputActionMappingDeviceTarget mActionMappingDeviceTarget = InputActionMappingDeviceTarget::INPUT_ACTION_MAPPING_TARGET_CONTROLLER;

        uint32_t mActionId = UINT_MAX;

        int32_t mDeviceButtons[4] = {0};
        
        uint8_t mUserId = 0;
    };


    class CYBER_INPUTSYSTEM_API InputSystem : public gainput::InputListener
    {
    public:
        InputSystem();

        ~InputSystem();

        void initInputSystem();
        void updateInputSystem(void* window);
        void addInputAction(const InputActionDesc& pDesc, const InputActionMappingDeviceTarget actionMappingTarget = InputActionMappingDeviceTarget::INPUT_ACTION_MAPPING_TARGET_ALL);
        void addDefaultActionMappings();
        virtual bool OnDeviceButtonBool(gainput::DeviceId device, gainput::DeviceButtonId deviceButton, bool oldValue, bool newValue) override;
    private:
        bool IsPointerType(gainput::DeviceId device) const;
        void AddActionMappings(const eastl::vector<ActionMappingDesc> actionMappings, const uint32_t numActions, const InputActionMappingDeviceTarget actionMappingTarget);
        void AddInputAction(const InputActionDesc& pDesc, const InputActionMappingDeviceTarget actionMappingTarget);
        void CreateActionForActionMapping(const ActionMappingDesc& pActionMappingDesc, const InputActionDesc& pActionDesc);
        static bool KeyChecker(InputActionContext* ctx);

        template<typename T>
        T* AllocateControl(const gainput::DeviceId deviceId)
        {
            T* pControl = (T*)mi_calloc(1, sizeof(T));
            auto& Pool = mControlPool[deviceId];
            Pool.push_back(*pControl);
            return pControl;
        }
    private:
        //Scope<gainput::InputManager> manager;
        Scope<gainput::InputMap> map;
        gainput::DeviceId mouseId;
        gainput::DeviceId keyboradId;
        gainput::DeviceId padId;

        const uint32_t MAX_INPUT_ACTIONS = 256;
        const uint32_t MAX_INPUT_GAMEPADS = 4;
        const uint32_t MAX_DEVICES = 16;

        /// Maps the action mapping ID to the ActionMappingDesc
        eastl::vector<eastl::vector<ActionMappingDesc>> mInputActionMappingIdToDesc;
        /// List of all input controls per device
        eastl::vector<eastl::vector<eastl::vector<IControl>>> mControls;
	    /// This global action will be invoked everytime there is a text character typed on a physical / virtual keyboard
        GlobalInputActionDesc mGlobalTextInputControl = {GlobalInputActionDesc::GlobalInputActionType::TEXT, nullptr, nullptr};
    	/// This global action will be invoked everytime there is a button action mapping triggered
        GlobalInputActionDesc mGlobalAnyButtonAction = {GlobalInputActionDesc::GlobalInputActionType::ANY_BUTTON_ACTION, nullptr, nullptr};

        /// List of controls which need to be canceled at the end of the frame
        eastl::vector<FloatControlSet> mFloatDeltaControlCancelQueue;
        eastl::vector<IControlSet> mButtonControlPerformQueue;

        eastl::map<gainput::DeviceId, eastl::vector<IControl>> mControlPool;

        DirectX::XMFLOAT2 mMousePosition;
        /// Gainput Manager which lets us talk with the gainput backend
        Scope<gainput::InputManager> pInputManager = nullptr;
       
        eastl::vector<InputDeviceType> pDeviceTypes;
        gainput::DeviceId* pGamepadDeviceIDs = nullptr;
        gainput::DeviceId mMouseDeviceID;
        gainput::DeviceId mRawMouseDeviceID;
        gainput::DeviceId mKeyboardDeviceID;
        gainput::DeviceId mTouchDeviceID;

        bool mVirtualKeyboardActive = false;
        bool mInputCaptured = false;
        bool mDefaultCapture = false;

    };
}