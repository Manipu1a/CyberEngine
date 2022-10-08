set_project("CyberEngine")

target("Engine")
set_kind("binary")
add_includedirs(include_dir_list, {public = true})
add_files(source_list)
add_files("src/*.cpp")