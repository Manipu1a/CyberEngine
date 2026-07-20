# RenderDoc Plugin

This directory is a local RenderDoc integration point for CyberEngine.

CyberEngine looks here first at runtime:

```text
Engine/Plugins/RenderDoc/bin/renderdoc.dll
```

If the DLL exists, it is loaded before D3D12 initialization so RenderDoc can hook graphics calls early. The overlay is disabled during startup. Press `F3` in the running sample to launch and attach the RenderDoc replay UI to the current process.

## Build RenderDoc

Run from the repository root:

```powershell
powershell -ExecutionPolicy Bypass -File Engine/Plugins/RenderDoc/build_renderdoc.ps1 -Configuration Release -Platform x64
```

The script clones or updates `https://github.com/baldurk/renderdoc.git` into `Engine/Plugins/RenderDoc/source`, builds `renderdoc.sln` with MSBuild, then copies the relevant binaries and runtime dependencies into `Engine/Plugins/RenderDoc/bin`.

By default the script passes `/p:PlatformToolset=v143` so the VS2015 RenderDoc solution can build with Visual Studio 2022. Override it with `-PlatformToolset` if you install a different toolset.

Use `-Fresh` to delete and re-clone the RenderDoc source directory before building.
Use `-SkipBuild` to refresh `bin/` from an already-built `source/x64/Release` directory.

## Runtime Overrides

- `CYBER_RENDERDOC_DLL`: load a specific `renderdoc.dll` instead of the plugin or installed copy.
- `CYBER_RENDERDOC_ENABLE=0`: disable automatic RenderDoc loading.

The RenderDoc source and copied binaries are intentionally ignored by Git.
