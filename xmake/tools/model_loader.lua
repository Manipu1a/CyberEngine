
--texture_loader_interface_dir = "$(projectdir)/tools/TextureLoader/interface"
model_loader_include_dir = "$(projectdir)/tools/ModelLoader/include"

target("ModelLoader")
    set_kind("static")
    add_files("$(projectdir)/tools/ModelLoader/src/model_loader.cpp")
    add_includedirs(model_loader_include_dir, {public=true})
    add_deps("CyberCore", {public = true})
    add_deps("CyberRuntime", {public = true})
    add_deps("CyberLog", {public = true})
    add_deps("mozjpeg", {public=true})
    add_deps("libpng", {public=true})
    add_deps("GLTF", {public=true})
    add_deps("TextureLoader", {public=true})
    add_defines("CYBER_API_EXPORT")
    add_defines( "TINYGLTF_NOEXCEPTION", "JSON_NOEXCEPTION",
                 "TINYGLTF_NO_STB_IMAGE_WRITE", "TINYGLTF_IMPLEMENTATION")