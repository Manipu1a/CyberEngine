target("renderpass")
    set_kind("binary")
    add_includedirs("include", {public = true})
    add_files("src/renderpass.cpp")
    add_deps("CyberRuntime", {public = false})