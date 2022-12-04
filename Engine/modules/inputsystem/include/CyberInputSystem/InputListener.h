#pragma once

#include <gainput/gainput.h>
#include "CyberLog/Log.h"

namespace Cyber
{
    class DeviceInputListener : public gainput::InputListener
    {
    public:
        DeviceInputListener(gainput::InputManager& manager, int32_t index): mManager(manager), mIndex(index) { }

        bool OnDeviceButtonBool(gainput::DeviceId deviceId, gainput::DeviceButtonId deviceButton, bool oldValue, bool newValue)
	    {
		    const gainput::InputDevice* device = mManager.GetDevice(deviceId);
		    char buttonName[64] = "";
		    device->GetButtonName(deviceButton, buttonName, 64);
            CB_CORE_INFO("{0} Device {1} ({2}{3}) bool button ({4}/{5}) changed: {6} -> {7}\n", mIndex, deviceId, device->GetTypeName(), device->GetIndex(), deviceButton, buttonName, oldValue, newValue);
		    //SFW_LOG();
		    return false;
	    }

	    bool OnDeviceButtonFloat(gainput::DeviceId deviceId, gainput::DeviceButtonId deviceButton, float oldValue, float newValue)
	    {
	    	const gainput::InputDevice* device = mManager.GetDevice(deviceId);
	    	char buttonName[64] = "";
	    	device->GetButtonName(deviceButton, buttonName, 64);
	    	CB_CORE_INFO("({0}) Device {1} ({2}{3}) float button ({4}/{5}) changed: {6} -> {7}\n", mIndex, deviceId, device->GetTypeName(), device->GetIndex(), deviceButton, buttonName, oldValue, newValue);
	    	return true;
	    }

        int32_t GetPriority() const
        {
            return mIndex;
        }

    private:
        gainput::InputManager& mManager;
        int32_t mIndex;
    };
}