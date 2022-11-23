set_project("CyberEngine")

target("Engine")
set_kind("binary")
set_languages("cxx17")

add_includedirs("$(projectdir)/Engine/include")
add_includedirs("$(projectdir)/Engine/include/Module")
add_includedirs(include_dir_list, {public = true})
add_files(source_list)
add_files("$(projectdir)/Engine/src/*.cpp")
add_files("$(projectdir)/Engine/src/Core/*.cpp")
add_files("$(projectdir)/Engine/src/Platform/*.cpp")
add_files("$(projectdir)/Engine/src/Module/Log/*.cpp")
add_files("$(projectdir)/Engine/src/Module/InputSystem/*.cpp")
set_pcheader("$(projectdir)/Engine/include/cepch.h")
add_defines( "GLFW_EXPOSE_NATIVE_WIN32","CB_DEBUG")
add_deps("GLFW", {public = true})
add_deps("Gainput", {public = true})
if (is_os("windows")) then 
        add_links("advapi32","gdi32","user32", "Shell32",{public = true})
end


