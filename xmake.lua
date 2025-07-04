set_project("CyberEngine")
add_rules("mode.debug", "mode.release", "mode.releasedbg")

include_dir_list = {"include"}
source_list = {}
packages_list = {}
defs_list = {}
links_list = {}
generator_list = {}

set_toolchains("msvc", {vs = "2022"})
add_cxxflags("/EHsc", {force = true})
set_languages("cxx20")
add_defines("UNICODE", "_UNICODE")
add_defines("_WINDOWS")
add_defines("CYBER_RUNTIME_PLATFORM_WINDOWS")
includes("xmake/xmake.lua")
includes("Engine/xmake.lua")
includes("samples/xmake.lua")
includes("xmake/options.lua")
if (is_os("windows")) then 
    add_links("advapi32","gdi32","user32", "Shell32", "dxcompiler", "d3dcompiler",{public = true})
    add_linkdirs("$(projectdir)/tools/DirectXShaderCompiler/lib/x64")
end



--
-- If you want to known more usage about xmake, please see https://xmake.io
--
-- ## FAQ
--
-- You can enter the project directory firstly before building project.
--
--   $ cd projectdir
--
-- 1. How to build project?
--
--   $ xmake
--
-- 2. How to configure project?
--
--   $ xmake f -p [macosx|linux|iphoneos ..] -a [x86_64|i386|arm64 ..] -m [debug|release]
--
-- 3. Where is the build output directory?
--
--   The default output directory is `./build` and you can configure the output directory.
--
--   $ xmake f -o outputdir
--   $ xmake
--
-- 4. How to run and debug target after building project?
--
--   $ xmake run [targetname]
--   $ xmake run -d [targetname]
--
-- 5. How to install target to the system directory or other output directory?
--
--   $ xmake install
--   $ xmake install -o installdir
--
-- 6. Add some frequently-used compilation flags in xmake.lua
--
-- @code
--    -- add debug and release modes
--    add_rules("mode.debug", "mode.release")
--
--    -- add macro defination
--    add_defines("NDEBUG", "_GNU_SOURCE=1")
--
--    -- set warning all as error
--    set_warnings("all", "error")
--
--    -- set language: c99, c++11
--    set_languages("c99", "c++11")
--
--    -- set optimization: none, faster, fastest, smallest
--    set_optimize("fastest")
--
--    -- add include search directories
--    add_includedirs("/usr/include", "/usr/local/include")
--
--    -- add link libraries and search directories
--    add_links("tbox")
--    add_linkdirs("/usr/local/lib", "/usr/lib")
--
--    -- add system link libraries
--    add_syslinks("z", "pthread")
--
--    -- add compilation and link flags
--    add_cxflags("-stdnolib", "-fno-strict-aliasing")
--    add_ldflags("-L/usr/local/lib", "-lpthread", {force = true})
--
-- @endcode
--

