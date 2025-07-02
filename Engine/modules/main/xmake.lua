
target("Main")
    set_kind("binary")
    --add_deps("triangle", {public = true})
    add_deps("cube", {public = true})
    add_files("main.cpp")
