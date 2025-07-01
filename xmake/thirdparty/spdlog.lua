spdlogincludes_dir = "$(projectdir)/thirdparty/spdlog/include"

--table.insert(include_dir_list, spdlogincludes_dir)

target("spdlog")
    set_kind("headeronly")
    add_includedirs(spdlogincludes_dir, {public=true})
    --add_files("$(projectdir)/thirdparty/spdlog/src/*.cpp")
    add_defines("FMT_UNICODE=0")
    add_defines("SPDLOG_COMPILED_LIB")