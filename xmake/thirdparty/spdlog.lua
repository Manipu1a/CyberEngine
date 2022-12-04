spdlogincludes_dir = "$(projectdir)/thirdparty/spdlog"

--table.insert(include_dir_list, spdlogincludes_dir)

target("spdlog")
    set_kind("headeronly")
    add_includedirs(spdlogincludes_dir, {public=true})