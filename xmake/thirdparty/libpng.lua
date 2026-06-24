libpng_includes_dir = "$(projectdir)/thirdparty/libpng"

target("libpng")
    set_kind("static")
    before_build(function (target)
        local libpng_dir = path.join(os.projectdir(), "thirdparty", "libpng")
        local build_dir = path.join(libpng_dir, "build")
        os.mkdir(build_dir)
        os.cp(path.join(libpng_dir, "scripts", "pnglibconf.h.prebuilt"), path.join(build_dir, "pnglibconf.h"))
    end)
    add_includedirs(libpng_includes_dir.."/build", {public=true})
    add_includedirs(libpng_includes_dir, {public=true})
    add_files(libpng_includes_dir.."/*.c")
    add_deps("zlib", {public=true})
