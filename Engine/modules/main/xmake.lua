
target("Main")
    set_kind("binary")
    --add_deps("triangle", {public = true})
    --add_deps("cube", {public = true})
    add_files("main.cpp")
    add_deps("CyberCore", {public = true})
    add_deps("CyberRuntime", {public = true})
    add_deps("pbr_demo", {public = true})
