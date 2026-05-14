
target("Main")
    set_kind("binary")
    add_files("main.cpp", "sample_registry.cpp", "sample_selector.cpp")
    add_deps("CyberCore", {public = true})
    add_deps("CyberRuntime", {public = true})
    add_syslinks("user32", "gdi32", "comctl32")

    -- Link every sample whose flag is set. If no flag is set at all,
    -- link every known-working runtime-loadable sample so the startup
    -- selector can present them. `renderpass` is a standalone binary
    -- and is never linked into Main. `cube` and `pbrdemo` currently
    -- fail to compile, and `triangle` fails at D3D12 resource creation;
    -- all three are opt-in only (see samples/xmake.lua for details).
    local _any_sample_flag = has_config("build_triangle")
        or has_config("build_cube")
        or has_config("build_pbrdemo")
        or has_config("build_shadow")
        or has_config("build_sponza")
    local _all = not _any_sample_flag

    if has_config("build_triangle") then
        add_deps("triangle", {public = true})
        add_defines("CYBER_HAS_TRIANGLE")
    end

    if has_config("build_cube") then
        add_deps("cube", {public = true})
        add_defines("CYBER_HAS_CUBE")
    end

    if has_config("build_pbrdemo") then
        add_deps("pbr_demo", {public = true})
        add_defines("CYBER_HAS_PBRDEMO")
    end

    if _all or has_config("build_shadow") then
        add_deps("shadow_sample", {public = true})
        add_defines("CYBER_HAS_SHADOW")
    end

    if _all or has_config("build_sponza") then
        add_deps("sponza_sample", {public = true})
        add_defines("CYBER_HAS_SPONZA")
    end
