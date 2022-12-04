glfw_includes_dir = "$(projectdir)/thirdparty/GLFW/include"
glfw_src_dir = "$(projectdir)/thirdparty/GLFW/src"
table.insert(include_dir_list, glfw_includes_dir)

target("GLFW")
set_kind("static")
add_defines("_GLFW_WIN32")
add_includedirs(glfw_includes_dir, {public=true})
add_files(glfw_src_dir.."/context.c",
        glfw_src_dir.."/init.c",
        glfw_src_dir.."/input.c",
        glfw_src_dir.."/monitor.c",
        glfw_src_dir.."/vulkan.c",
        glfw_src_dir.."/window.c"
    )

if (is_os("windows")) then
    add_files(glfw_src_dir.."/win32_init.c",
        glfw_src_dir.."/win32_joystick.c",
        glfw_src_dir.."/win32_monitor.c",
        glfw_src_dir.."/win32_time.c",
        glfw_src_dir.."/win32_thread.c",
        glfw_src_dir.."/win32_window.c",
        glfw_src_dir.."/wgl_context.c",
        glfw_src_dir.."/egl_context.c",
        glfw_src_dir.."/osmesa_context.c"
    )
end 

