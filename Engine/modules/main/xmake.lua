
target("Main")
    set_kind("binary")
    add_files("main.cpp")
    add_deps("CyberCore", {public = true})
    add_deps("CyberRuntime", {public = true})
    
    -- 根据配置选项动态添加示例依赖（只能启用一个）
    local sample_count = 0
    local active_sample = nil
    
    if has_config("build_triangle") then
        sample_count = sample_count + 1
        active_sample = "triangle"
    end
    
    if has_config("build_cube") then
        sample_count = sample_count + 1
        active_sample = "cube"
    end
    
    if has_config("build_pbrdemo") then
        sample_count = sample_count + 1
        active_sample = "pbr_demo"
    end
    
    if has_config("build_renderpass") then
        sample_count = sample_count + 1
        active_sample = "renderpass"
    end
    
    if has_config("build_shadow") then
        sample_count = sample_count + 1
        active_sample = "shadow_sample"
    end
    
    -- 确保只有一个示例被启用
    if sample_count > 1 then
        print("Warning: Multiple samples enabled. Only one sample can be active at a time for Main executable.")
        print("Please enable only one sample using --build_xxx=y option")
    end
    
    if active_sample then
        add_deps(active_sample, {public = true})
    end
