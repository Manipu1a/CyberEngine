--includes("samplebase/xmake.lua")
--includes("triangle/xmake.lua")
includes("cube/xmake.lua")

if has_config("build_triangle") then
    includes("triangle/xmake.lua")
end

if has_config("build_cube") then
    includes("cube/xmake.lua")
end

if has_config("build_renderpass") then
    --includes("renderpass/xmake.lua")
end

