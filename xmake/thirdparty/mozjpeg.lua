--mozjpeg_sources_dir = "$(projectdir)/thirdparty/mozjpeg"
mozjpeg_includes_dir = "$(projectdir)/thirdparty/mozjpeg/include"

target("mozjpeg")
set_kind("phony")
add_includedirs(mozjpeg_includes_dir, {public=true})
add_linkdirs("$(projectdir)/thirdparty/mozjpeg/lib")
add_links( "jpeg", "jpeg-static","turbojpeg-static", "turbojpeg",  {public = true})