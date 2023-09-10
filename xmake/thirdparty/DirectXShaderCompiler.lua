dxc_includes_dir = "$(projectdir)/thirdparty/DirectXShaderCompiler"

table.insert(include_dir_list, dxc_includes_dir.."/dxc")
table.insert(include_dir_list, dxc_includes_dir.."/dxc/DXIL")
table.insert(include_dir_list, dxc_includes_dir.."/dxc/DxilContainer")
table.insert(include_dir_list, dxc_includes_dir.."/dxc/Support")
add_files(dxc_includes_dir.."/dxc/Support/*.cpp")