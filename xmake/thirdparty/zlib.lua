zlib_inlcudes_dir = "$(projectdir)/thirdparty/zlib"

target("zlib")
    set_kind("static")
    add_includedirs(zlib_inlcudes_dir, {public=true})
    add_includedirs(zlib_inlcudes_dir.."/build", {public=true})
    add_files(zlib_inlcudes_dir.."/*.c")
