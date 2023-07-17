target("Main")
    set_kind("binary")
    add_deps("CyberRuntime", {public = true})
    add_files("main.cpp")
