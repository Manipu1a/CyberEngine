target("CyberApplication")
    set_kind("shared")
    add_includedirs("include", {public=true})
    add_deps("CyberRuntime", {public=true})
    add_deps("triangle", {public = true})