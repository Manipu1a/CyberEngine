target("CyberRuntime")
    set_kind("shared")
    add_includedirs("include", {public=true})
    add_files("src/core/*.cpp")
    add_files("src/platform/*.cpp")
    add_deps("EASTL", {public = true})
    add_deps("CyberLog", {public = true})
    add_deps("CyberEvents",{public=true})