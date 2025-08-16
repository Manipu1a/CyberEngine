#include "inputsystem/core/input_manager.h"
#include "inputsystem/devices/keyboard_device.h"
#include "inputsystem/devices/mouse_device.h"

namespace CyberEngine::Input {
    class InputBackendWin32 : public InputManager {
    public:
        InputBackendWin32();
        virtual ~InputBackendWin32();

        virtual void process_platform_event(void* nativeEvent) override;
    };
}