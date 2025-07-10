
--texture_loader_interface_dir = "$(projectdir)/tools/TextureLoader/interface"
texture_loader_include_dir = "$(projectdir)/tools/TextureLoader/include"

target("TextureLoader")
    set_kind("static")
    add_files("$(projectdir)/tools/TextureLoader/src/*.cpp")
    add_files("$(projectdir)/tools/TextureLoader/src/texture_utils.cpp")
    add_includedirs(texture_loader_include_dir, {public=true})
    add_deps("CyberCore", {public = true})
    add_deps("CyberRuntime", {public = true})
    add_deps("CyberLog", {public = true})
    add_deps("mozjpeg", {public=true})
    add_deps("libpng", {public=true})
    add_deps("stb", {public=true})
    add_defines("CYBER_API_EXPORT")