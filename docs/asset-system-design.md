# CyberEngine 资源系统设计方案

## 目标

CyberEngine 不应让编辑器、PIE 或游戏运行时直接依赖 FBX、PNG、WAV 等原始资源格式。原始资源只作为导入输入和重新导入依据，进入引擎后应转换为引擎自己的资产格式、运行时格式和打包缓存。

这个资源系统需要支持：

- 编辑器资源浏览、预览、属性编辑和重新导入。
- PIE 尽量使用接近真实游戏运行时的加载路径。
- 增量 cook、平台 cook、资源依赖追踪和版本升级。
- 打包后的资源读取、异步加载、流式加载和卸载。
- Runtime Object 与磁盘资产解耦，避免 GPU 资源和文件格式互相污染。

## 推荐分层

```text
Source Files
FBX / PNG / WAV / glTF / PSD / HDR
        |
        | import
        v
Editor Assets
.meshasset / .textureasset / .material / .scene / .animasset
跨平台、可编辑、带 GUID、元数据、导入设置、依赖关系、预览数据
        |
        | derive / build
        v
Derived Data Cache
可删除、可重建，用于缓存压缩纹理、优化 mesh、压缩动画、shader variant 等中间结果
        |
        | cook
        v
Cooked Runtime Assets
.meshbin / .texbin / .matbin / .animbin / .scenebin
平台相关、运行时友好、移除编辑器数据、支持异步和流式加载
        |
        | package
        v
Runtime Packages
.pak / .bundle / .assetpack
资源包、索引、压缩块、chunk、依赖表、平台标记
        |
        | async load
        v
Runtime Objects
Texture / Mesh / Material / Animation / Scene / GPU Resource
真正参与游戏运行的数据对象
```

## 各层职责

### 1. Source Files

Source Files 是艺术家和外部工具生产的原始文件，例如：

- `fbx`, `gltf`, `obj`
- `png`, `tga`, `hdr`, `psd`
- `wav`, `ogg`
- 外部 DCC 工具导出的场景、动画、材质描述

这一层不应被 runtime 直接读取。编辑器也不应把它当作资产系统的主要数据源，只能用于：

- 首次导入。
- Reimport。
- 检查源文件 hash 是否变化。
- 保留原始路径、导入器版本和导入设置。

### 2. Editor Assets

Editor Assets 是编辑器内的权威资产格式。导入 FBX、PNG、WAV 后，编辑器应该生成引擎专属资产文件，例如：

- `.meshasset`
- `.textureasset`
- `.material`
- `.scene`
- `.animasset`
- `.skeleton`

Editor Asset 应该是跨平台的、可编辑的、可序列化的。它可以包含比 runtime 更多的信息。

建议保存内容：

- 稳定 `AssetGuid`。
- `AssetType`。
- 原始文件路径。
- 原始文件 hash。
- 导入器名称和导入器版本。
- 导入设置。
- 资产元数据。
- 子资源列表。
- 依赖关系。
- 编辑器预览数据。
- 缩略图。
- 用户可编辑数据。
- 资产格式版本。

示例：

```text
character.fbx
    -> character.scene
    -> character_body.meshasset
    -> character_skeleton.skeleton
    -> walk.animasset
    -> character_body.material
```

编辑器资源浏览器、属性面板、资源预览窗口和场景编辑器都应该引用 Editor Assets，而不是直接引用原始 FBX 或 PNG。

### 3. Derived Data Cache

Derived Data Cache 是可删除、可重建的派生数据缓存。它不是权威资产，不应该被手动编辑，也不应该成为项目中唯一保存的数据。

DDC 的 key 应该由以下信息组成：

- Editor Asset GUID。
- Editor Asset 内容 hash。
- 导入设置 hash。
- 平台。
- cooker 版本。
- 编译器版本。
- 相关依赖 hash。

适合放入 DDC 的内容：

- 压缩纹理结果，例如 BC1、BC3、BC5、BC7。
- mesh 优化结果，例如顶点重排、index 优化、meshlet、LOD。
- 动画压缩结果。
- shader permutation 编译结果。
- 场景加速结构中间数据。
- 材质编译中间结果。

DDC 可以显著加快编辑器预览和增量 cook。删除 DDC 后，系统应该可以从 Editor Assets 重新生成。

### 4. Cooked Runtime Assets

Cooked Runtime Assets 是面向具体平台的运行时资产文件，例如：

- `.meshbin`
- `.texbin`
- `.matbin`
- `.animbin`
- `.scenebin`

这一层应该移除编辑器专用数据，只保留运行时需要的数据，并尽量使用运行时友好的布局。

Cooked Runtime Asset 的特点：

- 平台相关。
- 格式紧凑。
- 可快速读取。
- 可异步加载。
- 可流式加载。
- 可被 Runtime Package 打包。
- 可以从 Editor Asset 和 DDC 重新生成。

示例：

```text
.textureasset
    -> Windows-D3D12/.texbin: BC7 mip chain, texture metadata

.meshasset
    -> Windows-D3D12/.meshbin: vertex stream, index stream, submesh table, bounds
```

Cooked Runtime Asset 不应该直接保存 D3D12 对象。它保存的是能创建 runtime object 和 GPU resource 的二进制数据。

### 5. Runtime Packages

Runtime Package 是打包后的资源容器，例如：

- `.pak`
- `.bundle`
- `.assetpack`

它负责组织多个 Cooked Runtime Assets，并提供运行时快速定位和读取能力。

建议 Runtime Package 包含：

- Package header。
- Asset index。
- Asset GUID 到 package offset 的映射。
- 压缩块信息。
- chunk 信息。
- 依赖表。
- 平台标记。
- 内容 hash。
- 加密或签名信息，可后续加入。

Runtime Package 和 Cooked Runtime Asset 的关系：

- Cooked Runtime Asset 是单个资源的运行时二进制结果。
- Runtime Package 是多个 Cooked Runtime Assets 的装箱和索引。

开发阶段可以直接从 loose cooked files 读取。正式打包时再写入 package。

### 6. Runtime Objects

Runtime Object 是游戏运行时真正使用的内存对象。它不是磁盘格式。

示例：

```text
.texbin
    -> TextureResource
    -> D3D12 texture
    -> SRV descriptor

.meshbin
    -> MeshResource
    -> vertex buffer
    -> index buffer
    -> optional BLAS input

.matbin
    -> MaterialResource
    -> pipeline state reference
    -> descriptor set bindings
```

Runtime Object 需要支持：

- 引用计数或 handle 生命周期。
- 异步加载状态。
- GPU upload 状态。
- 热重载。
- 资源卸载。
- 设备丢失或 renderer 重建时的恢复。

Runtime Object 不应反向依赖 Editor Asset，也不应知道 FBX、PNG 等原始文件格式。

## Asset Registry

Asset Registry 是资源系统的核心索引。没有它，编辑器浏览、依赖分析、cook 和运行时加载都会变得脆弱。

Asset Registry 应记录：

- `AssetGuid`。
- 资产路径。
- 资产类型。
- 显示名。
- 源文件路径。
- 源文件 hash。
- Editor Asset hash。
- Cooked Asset hash。
- 导入器版本。
- 资产格式版本。
- 依赖列表。
- 被引用关系，可选。
- 平台 cook 状态。
- 所属 package 或 chunk。
- 缩略图路径或缩略图缓存 key。

推荐使用稳定 GUID 作为资产引用的核心，而不是使用文件路径作为唯一身份。

路径可以变化，GUID 不应变化。重命名和移动资产时，只更新 Registry 和资产路径，不改变引用关系。

## Asset Reference

资产引用建议分为两类：

```cpp
struct AssetId
{
    Guid guid;
};

struct SoftAssetRef
{
    Guid guid;
    AssetType expectedType;
};
```

编辑器里可以显示路径和名字，但序列化引用时应保存 GUID。

Runtime 可以通过 GUID 查找 package index，然后异步加载对应资源。

## 文件头建议

每种二进制资产格式都应有明确 header。

```cpp
struct AssetFileHeader
{
    uint32_t magic;
    uint32_t formatVersion;
    uint32_t assetType;
    Guid assetGuid;
    uint64_t contentHash;
    uint32_t cookerVersion;
    uint32_t platformTag;
    uint32_t dependencyHash;
    uint32_t payloadOffset;
    uint32_t payloadSize;
};
```

这样可以支持：

- 格式升级。
- 资产类型校验。
- 平台校验。
- 缓存失效。
- 依赖变化检测。
- 快速跳过不兼容文件。

## 导入流程

```text
用户导入 source file
        |
        v
选择 importer
        |
        v
读取 source file
        |
        v
生成 editor asset 和 sub assets
        |
        v
写入 asset files
        |
        v
更新 Asset Registry
        |
        v
生成缩略图和预览缓存
```

Importer 应该是插件化的：

- `FbxImporter`
- `TextureImporter`
- `AudioImporter`
- `MaterialImporter`
- `SceneImporter`

每个 importer 负责把外部格式转换为 Editor Asset。导入器不应直接生成 D3D12 resource。

## Cook 流程

```text
选择目标平台
        |
        v
扫描 Asset Registry
        |
        v
构建依赖图
        |
        v
检查 Editor Asset hash / importer version / cooker version
        |
        v
命中 DDC 则复用
        |
        v
未命中则重新 derive
        |
        v
生成 Cooked Runtime Assets
        |
        v
写入 loose cooked files 或 Runtime Package
        |
        v
更新 cook manifest
```

Cook 应该支持：

- 全量 cook。
- 增量 cook。
- 按平台 cook。
- 按目标配置 cook，例如 DebugGame、Development、Shipping。
- cook-on-the-fly，用于 PIE 或远程调试。

## PIE 策略

PIE 不建议直接加载 Editor Asset 并绕过 runtime 资源路径。否则编辑器能运行但打包后可能失败。

推荐策略：

- 编辑器编辑态使用 Editor Asset。
- 点击 PIE 时触发增量 cook 或 cook-on-the-fly。
- PIE 通过 runtime loader 加载 Cooked Runtime Asset。
- 未 cook 或 cook 过期时自动生成。

这样可以让 PIE 更接近真实游戏运行环境。

## Runtime 加载流程

```text
请求 AssetId
        |
        v
查询 Runtime Asset Registry / Package Index
        |
        v
检查依赖
        |
        v
异步读取 package block 或 loose cooked file
        |
        v
解压 / 解码
        |
        v
创建 CPU-side Runtime Object
        |
        v
提交 GPU upload
        |
        v
完成资源状态切换
        |
        v
返回可用 handle
```

Runtime loader 应该支持：

- 异步 IO。
- 依赖自动加载。
- 加载优先级。
- streaming。
- 卸载。
- fallback resource。
- 热重载。

## 建议的目录结构

项目目录可以采用类似结构：

```text
Project/
    Content/
        Source/
            Characters/hero.fbx
            Textures/hero_albedo.png

        Assets/
            Characters/hero.scene
            Characters/hero_body.meshasset
            Materials/hero.material
            Textures/hero_albedo.textureasset

        Registry/
            AssetRegistry.json

    Saved/
        DerivedDataCache/
            Windows-D3D12/

        Cooked/
            Windows-D3D12/
                loose cooked files

        Packages/
            Windows-D3D12/
                game.assetpack
```

对于 CyberEngine 当前仓库，可以先从引擎侧建立通用模块，再决定项目内容目录最终放在样例工程还是独立 game project 下。

## 建议的引擎模块

可以在 `CyberRuntime` 中逐步加入：

```text
asset/
    AssetId.h
    AssetTypes.h
    AssetRegistry.h/.cpp
    AssetDatabase.h/.cpp
    AssetImporter.h
    AssetCooker.h
    AssetPackage.h/.cpp
    AssetLoader.h/.cpp
    RuntimeResource.h
```

编辑器相关代码可以单独放在：

```text
editor/asset/
    AssetBrowserPanel
    AssetInspectorPanel
    AssetPreviewPanel
    ImportDialog
    ReimportTools
```

导入器可以独立组织：

```text
asset/importers/
    TextureImporter
    MeshImporter
    SceneImporter
    AudioImporter
```

Cooker 可以按类型拆分：

```text
asset/cookers/
    TextureCooker
    MeshCooker
    MaterialCooker
    AnimationCooker
    SceneCooker
```

## 序列化格式建议

Editor Asset 推荐优先使用可调试格式或混合格式：

- JSON / YAML + binary blob。
- 或自定义 chunked binary，但提供 dump 工具。

Cooked Runtime Asset 推荐使用 chunked binary：

```text
Header
Chunk Table
Metadata Chunk
Dependency Chunk
Payload Chunk
Optional Streaming Chunks
```

Runtime Package 推荐使用：

```text
Package Header
Package Index
Compressed Blocks
Asset Payloads
Dependency Table
String Table
```

## 依赖管理

资源依赖必须显式记录。示例：

```text
Scene
    -> Mesh
    -> Material
        -> Texture
        -> Shader
    -> Skeleton
    -> Animation
```

Cook 时需要根据依赖图决定：

- cook 顺序。
- package 归属。
- chunk 归属。
- 是否需要重新 cook。
- runtime 加载时是否自动拉起依赖。

## 版本和缓存失效

任何影响输出的内容都必须进入 hash 或版本号。

应该影响 cook 缓存的因素：

- Editor Asset 内容变化。
- Source File hash 变化。
- Import Settings 变化。
- Importer version 变化。
- Cooker version 变化。
- Target platform 变化。
- Texture compression settings 变化。
- Mesh optimization settings 变化。
- Material shader permutation 变化。
- 依赖资产变化。

## 最小可落地版本

建议不要一开始做完整 package、streaming、远程 DDC。可以按阶段实现。

### Phase 1: Editor Asset 和 Registry

- 定义 `AssetGuid`。
- 定义基础 `AssetRegistry`。
- 支持 texture 和 mesh 的导入记录。
- 原始文件导入为 `.textureasset` 和 `.meshasset`。
- 编辑器不直接引用原始路径。

### Phase 2: Cooked Runtime Asset

- 实现 `.texbin`。
- 实现 `.meshbin`。
- Runtime loader 从 cooked 文件创建 runtime object。
- PIE 走 cooked 加载路径。

### Phase 3: DDC 和增量 cook

- 加入 DDC key。
- 加入 cook manifest。
- 支持 hash 检查和跳过未变化资产。

### Phase 4: Runtime Package

- loose cooked files 工作稳定后，再实现 `.assetpack`。
- 加入 package index。
- 支持按 GUID 查找 offset。
- 支持压缩块。

### Phase 5: Streaming 和高级能力

- 纹理 mip streaming。
- mesh LOD streaming。
- 异步依赖加载。
- 热重载。
- 资源卸载策略。

## 关键原则

- Source Files 只用于导入和 reimport。
- Editor Assets 是编辑器里的权威资产。
- DDC 是可删除、可重建的缓存。
- Cooked Runtime Assets 是平台相关的运行时资产。
- Runtime Packages 是打包和分发容器。
- Runtime Objects 是内存和 GPU 中真正参与游戏运行的数据。
- 资产引用应使用稳定 GUID，而不是路径。
- PIE 应尽量走 runtime 加载路径。
- 所有格式都必须版本化。
- 所有派生结果都必须可追踪、可失效、可重建。

