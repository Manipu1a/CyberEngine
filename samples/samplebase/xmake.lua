target("samples")
    set_kind("shared")
    add_includedirs("$(projectdir)/samples/samplebase", {public = true})
    add_files("*.cpp")
    add_deps("CyberRuntime", {public = true})
    add_defines("CYBER_SAMPLES_API_EXPORT")

