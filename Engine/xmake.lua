

--target("Engine")
--set_kind("binary")
--set_languages("cxx17")
add_files(source_list)
add_includedirs(include_dir_list, {public = true})
set_pcheader("$(projectdir)/Engine/cepch.h")
includes("Modules/xmake.lua")

