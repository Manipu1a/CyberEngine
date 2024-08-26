libpng_includes_dir = "$(projectdir)/thirdparty/libpng"

target("libpng")
    set_kind("static")
    add_includedirs(libpng_includes_dir.."/build", {public=true})
    add_includedirs(libpng_includes_dir, {public=true})
    add_files(libpng_includes_dir.."/*.c")
    add_deps("zlib", {public=true})