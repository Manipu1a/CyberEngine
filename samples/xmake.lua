--includes("samplebase/xmake.lua")
--includes("triangle/xmake.lua")
--includes("pbrdemo/xmake.lua")

-- If no --build_xxx flag is set at all, build every KNOWN-WORKING
-- runtime-loadable sample so the user can pick one via the startup
-- selector dialog. Explicit flags still win: setting one or more narrows
-- the set.
--
-- NOTE 1: `renderpass` is a standalone binary target and is intentionally
-- NOT part of the default-all set.
-- NOTE 2: `cube` and `pbrdemo` reference an older graphics API (e.g.
-- `m_format`, old `create_render_pipeline` signature) and currently fail
-- to compile. `triangle` compiles but crashes at runtime with
-- "Failed to create placed resource" against the current D3D12 resource
-- manager (latent bit-rot). All three stay opt-in via explicit flag
-- until they get updated; the default-all path skips them so the
-- selector flow works out of the box.
local _any_sample_flag = has_config("build_triangle")
    or has_config("build_cube")
    or has_config("build_pbrdemo")
    or has_config("build_renderpass")
    or has_config("build_shadow")
    or has_config("build_sponza")
local _all = not _any_sample_flag

if has_config("build_triangle") then
    includes("triangle/xmake.lua")
end

if has_config("build_cube") then
    includes("cube/xmake.lua")
end

if has_config("build_pbrdemo") then
    includes("pbrdemo/xmake.lua")
end

if has_config("build_renderpass") then
    includes("renderpass/xmake.lua")
end

if _all or has_config("build_shadow") then
    includes("shadow/xmake.lua")
end

if _all or has_config("build_sponza") then
    includes("sponza/xmake.lua")
end
