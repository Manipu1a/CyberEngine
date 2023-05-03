eastl_sources_dir = "$(projectdir)/thirdparty/EASTL/EASTL"
eastl_includes_dir = "$(projectdir)/thirdparty/EASTL"
--table.insert(source_list, eastl_sources_dir.."/eastl.cpp")
--table.insert(source_list, eastl_includes_dir.."/source/red_black_tree.cpp")
--table.insert(include_dir_list, eastl_includes_dir)

target("EASTL")
    set_kind("static")
    add_deps("mimalloc")
    add_includedirs(eastl_includes_dir, {public = true})
    add_files(eastl_sources_dir.."/eastl.cpp")
    add_files(eastl_sources_dir.."/allocator_cyber.cpp")
    add_files(eastl_sources_dir.."/red_black_tree.cpp")
    add_files(eastl_sources_dir.."/*.cpp")