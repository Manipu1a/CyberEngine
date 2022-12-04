mimalloc_sources_dir = "$(projectdir)/thirdparty/mimalloc"
mimalloc_includes_dir = "$(projectdir)/thirdparty/mimalloc/include"

target("mimalloc")
set_kind("static")
add_files(mimalloc_sources_dir.."/alloc.c")
add_files(mimalloc_sources_dir.."/alloc-aligned.c")
add_files(mimalloc_sources_dir.."/alloc-posix.c")
add_files(mimalloc_sources_dir.."/stats.c")
add_files(mimalloc_sources_dir.."/random.c")
add_files(mimalloc_sources_dir.."/os.c")
add_files(mimalloc_sources_dir.."/bitmap.c")
add_files(mimalloc_sources_dir.."/arena.c")
add_files(mimalloc_sources_dir.."/segment-cache.c")
add_files(mimalloc_sources_dir.."/segment.c")
add_files(mimalloc_sources_dir.."/page.c")
add_files(mimalloc_sources_dir.."/heap.c")
add_files(mimalloc_sources_dir.."/options.c")
add_files(mimalloc_sources_dir.."/init.c")

add_includedirs(mimalloc_includes_dir, {public=true})
