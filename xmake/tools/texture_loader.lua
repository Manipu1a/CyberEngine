
texture_loader_interface_dir = "$(projectdir)/tools/TextureLoader/interface"
texture_loader_include_dir = "$(projectdir)/tools/TextureLoader/include"

target("TextureLoader")
    set_kind("static")
    add_includedirs(texture_loader_include_dir, {public=true})
    add_includedirs(texture_loader_interface_dir, {public=true})
    add_files("$(projectdir)/tools/TextureLoader/src/*.cpp")
    add_deps("CyberRuntime", {public=true})
    add_deps("mozjpeg", {public=true})
    add_deps("libpng", {public=true})