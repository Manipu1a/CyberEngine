--mozjpeg_sources_dir = "$(projectdir)/thirdparty/mozjpeg"
mozjpeg_includes_dir = "$(projectdir)/thirdparty/mozjpeg/include/mozjpeg"
table.insert(include_dir_list, mozjpeg_includes_dir)

target("mozjpeg")
set_kind("headeronly")
add_includedirs(mozjpeg_includes_dir)
--add_links("jpeg", "jpeg-static","turbojpeg-static", "turbojpeg")
--add_linkdirs("$(projectdir)/thirdparty/mozjpeg/lib")