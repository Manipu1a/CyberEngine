gameruntime_includes_dir = "$(projectdir)/Engine/modules/application/include"
table.insert(include_dir_list, gameruntime_includes_dir)

target("GameRuntime")
    set_kind("shared")
    add_includedirs("include", {public=true})
    add_deps("CyberRuntime", {public=true})
    add_deps("CyberInputSystem", {public=true})
    add_files("src/game_runtime/*.cpp")