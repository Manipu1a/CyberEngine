gltf_includes_dir = "$(projectdir)/thirdparty/GLTF/include"
table.insert(include_dir_list, gltf_includes_dir)

target("GLTF")
set_kind("headeronly")
add_includedirs(gltf_includes_dir)