directxincludes_dir = "$(projectdir)/thirdparty/DirectX/include"

target("directx")
    set_kind("static")
    add_linkdirs("$(projectdir)/thirdparty/DirectX/lib/x64")
    add_includedirs(directxincludes_dir, {public=true})
