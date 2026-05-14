nlohmann_json_includes_dir = "$(projectdir)/thirdparty/nlohmann_json/include"

target("nlohmann_json")
    set_kind("headeronly")
    add_includedirs(nlohmann_json_includes_dir, {public=true})
