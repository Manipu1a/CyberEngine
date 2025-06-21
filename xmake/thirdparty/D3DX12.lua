d3dx12includes_dir = "$(projectdir)/thirdparty/D3DX12/include"

target("d3dx12")
    set_kind("headeronly")
    add_deps("directx", {public=true})
    add_includedirs(d3dx12includes_dir, {public=true})