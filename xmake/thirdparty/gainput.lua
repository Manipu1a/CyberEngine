gainput_includes_dir = "$(projectdir)/thirdparty/gainput/lib/include"
gainput_src_dir = "$(projectdir)/thirdparty/gainput/lib/source/gainput"
--table.insert(include_dir_list, gainput_includes_dir)

target("Gainput")
set_kind("static")
add_includedirs(gainput_includes_dir, {public = true})
add_defines("GAINPUT_LIB_DYNAMIC","_WINDOWS", "gainput_EXPORTS" ,"WIN32")
add_files(gainput_src_dir.."gainput.cpp",
gainput_src_dir.."/GainputAllocator.cpp",
gainput_src_dir.."/gestures/GainputButtonStickGesture.cpp",
gainput_src_dir.."/GainputDev.cpp",
gainput_src_dir.."/gestures/GainputHoldGesture.cpp",
gainput_src_dir.."/GainputinputDeltaState.cpp",
gainput_src_dir.."/GainputInputDevice.cpp",
gainput_src_dir.."/builtin/GainputInputDeviceBuiltIn.cpp",
gainput_src_dir.."/keyboard/GainputInputDeviceKeyboard.cpp",
gainput_src_dir.."/keyboard/GainputInputDeviceKeyboardMac.cpp",
gainput_src_dir.."/mouse/GainputInputDeviceMouse.cpp",
gainput_src_dir.."/pad/GainputInputDevicePad.cpp",
gainput_src_dir.."/pad/GainputInputDevicePadMac.cpp",
gainput_src_dir.."/touch/GainputInputDeviceTouch.cpp",
gainput_src_dir.."/GainputInputManager.cpp",
gainput_src_dir.."/GainputInputMap.cpp",
gainput_src_dir.."/recorder/GainputInputPlayer.cpp",
gainput_src_dir.."/recorder/GainputInputRecorder.cpp",
gainput_src_dir.."/recorder/GainputInputRecording.cpp",
gainput_src_dir.."/GainputInputState.cpp",
gainput_src_dir.."/GainputMemoryStream.cpp",
gainput_src_dir.."/GainputMapFilters.cpp",
gainput_src_dir.."/GainputNetAddress.cpp",
gainput_src_dir.."/GainputNetConnection.cpp",
gainput_src_dir.."/GainputNetListener.cpp",
gainput_src_dir.."/gestures/GainputPinchGesture.cpp",
gainput_src_dir.."/gestures/GainputRotateGesture.cpp",
gainput_src_dir.."/gestures/GainputSimultaneouslyDownGesture.cpp",
gainput_src_dir.."/gestures/GainputTapGesture.cpp",
gainput_src_dir.."/gestures/GainputDoubleClickGesture.cpp",
gainput_src_dir.."/dev/GainputMemoryStream.cpp"
        )
add_links("Xinput9_1_0","ws2_32", {public = true})
