#include "inputsystem/core/input_manager.h"
#include "inputsystem/devices/keyboard_device.h"
#include "inputsystem/devices/mouse_device.h"

namespace Cyber::Input {
    class InputBackendWin32 : public InputManager {
    public:
        virtual void process_platform_event(void* nativeEvent) override;
    };
}