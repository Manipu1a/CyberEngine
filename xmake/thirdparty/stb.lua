stbincludes_dir = "$(projectdir)/thirdparty/stb"

target("stb")
    set_kind("headeronly")
    add_includedirs(stbincludes_dir, {public=true})