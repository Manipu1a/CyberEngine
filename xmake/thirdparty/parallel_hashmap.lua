parallel_hashmap_includes_dir = "$(projectdir)/thirdparty/parallel_hashmap"

target("parallel_hashmap")
    set_kind("headeronly")
    add_includedirs(parallel_hashmap_includes_dir, {public=true})