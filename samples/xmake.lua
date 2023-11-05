includes("samplebase/xmake.lua")
if has_config("build_triangle") then
    includes("triangle/xmake.lua")
end

if has_config("build_renderpass") then
    includes("renderpass/xmake.lua")
end
