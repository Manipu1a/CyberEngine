imgui_includes_dir = "$(projectdir)/thirdparty/IMGUI"

target("IMGUI")
    set_kind("static")
    add_includedirs(imgui_includes_dir, {public=true})
    add_files(imgui_includes_dir.."/imgui/*.cpp")
    add_files(imgui_includes_dir.."/imgui/backends/imgui_impl_win32.cpp")
    add_files(imgui_includes_dir.."/imgui/backends/imgui_impl_dx12.cpp")
