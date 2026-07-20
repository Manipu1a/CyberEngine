#include "asset/asset.h"

#include <cassert>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

int main()
{
    using namespace Cyber;
    namespace fs = std::filesystem;

    const AssetGuid guid = AssetGuid::Create();
    assert(guid.IsValid());

    const std::string guidText = guid.ToString();
    assert(guidText.size() == 36);

    AssetGuid parsedGuid;
    assert(AssetGuid::FromString(guidText, parsedGuid));
    assert(parsedGuid == guid);

    AssetId id(guid);
    AssetId parsedId;
    assert(AssetId::FromString(id.ToString(), parsedId));
    assert(parsedId == id);

    SoftAssetRef textureRef(id, AssetType::Texture);
    assert(textureRef.IsValid());

    AssetType parsedType = AssetType::Unknown;
    assert(TryParseAssetType("texture", parsedType));
    assert(parsedType == AssetType::Texture);
    assert(std::strcmp(ToString(AssetType::Mesh), "Mesh") == 0);

    AssetFileHeader header;
    header.assetType = AssetType::Texture;
    header.assetGuid = guid;
    header.contentHash = AssetHash::HashString("source");
    header.dependencyHash = AssetHash::HashString("dependency");
    header.platformTag = kWindowsD3D12AssetPlatform;
    header.payloadOffset = sizeof(AssetFileHeader);
    header.payloadSize = 128;

    assert(header.IsValid());
    assert(header.IsCompatible(AssetType::Texture, kAssetFileFormatVersion, kWindowsD3D12AssetPlatform));
    assert(!header.IsCompatible(AssetType::Mesh, kAssetFileFormatVersion, kWindowsD3D12AssetPlatform));

    const uint64_t hashA = AssetHash::HashString("same");
    const uint64_t hashB = AssetHash::HashString("same");
    const uint64_t hashC = AssetHash::HashString("different");
    assert(hashA == hashB);
    assert(hashA != hashC);
    assert(AssetHash::Combine(hashA, hashC) == AssetHash::Combine(hashA, hashC));

    const fs::path testRoot =
        fs::current_path() / "Saved" / "AssetFoundationTests" / AssetGuid::Create().ToString();
    const fs::path contentRoot = testRoot / "Content";
    const fs::path sourcePath = testRoot / "Source" / "checker.png";
    const fs::path assetPath = contentRoot / "Assets" / "Textures" / "checker.textureasset";
    const fs::path meshSourcePath = testRoot / "Source" / "triangle.gltf";
    const fs::path meshAssetPath = contentRoot / "Assets" / "Meshes" / "cube.meshasset";

    fs::create_directories(sourcePath.parent_path());
    fs::create_directories(assetPath.parent_path());
    fs::create_directories(meshAssetPath.parent_path());

    const std::vector<uint8_t> fakePngBytes {
        0x89u, 0x50u, 0x4eu, 0x47u, 0x0du, 0x0au, 0x1au, 0x0au,
        0x43u, 0x59u, 0x42u, 0x45u, 0x52u
    };
    {
        std::ofstream sourceFile(sourcePath, std::ios::binary | std::ios::trunc);
        sourceFile.write(reinterpret_cast<const char*>(fakePngBytes.data()),
                         static_cast<std::streamsize>(fakePngBytes.size()));
        assert(sourceFile.good());
    }

    const std::string triangleGltf = R"({
        "asset":{"version":"2.0"},
        "buffers":[{"byteLength":42,"uri":"data:application/octet-stream;base64,AAAAAAAAAAAAAAAAAACAPwAAAAAAAAAAAAAAAAAAgD8AAAAAAAABAAIA"}],
        "bufferViews":[
            {"buffer":0,"byteOffset":0,"byteLength":36,"target":34962},
            {"buffer":0,"byteOffset":36,"byteLength":6,"target":34963}
        ],
        "accessors":[
            {"bufferView":0,"componentType":5126,"count":3,"type":"VEC3","min":[0,0,0],"max":[1,1,0]},
            {"bufferView":1,"componentType":5123,"count":3,"type":"SCALAR"}
        ],
        "meshes":[{"primitives":[{"attributes":{"POSITION":0},"indices":1}]}],
        "nodes":[{"mesh":0}],
        "scenes":[{"nodes":[0]}],
        "scene":0
    })";
    {
        std::ofstream sourceFile(meshSourcePath, std::ios::binary | std::ios::trunc);
        sourceFile.write(triangleGltf.data(), static_cast<std::streamsize>(triangleGltf.size()));
        assert(sourceFile.good());
    }

    TextureImporter importer;
    AssetImportRequest importRequest;
    importRequest.sourcePath = sourcePath;
    importRequest.destinationPath = assetPath;
    importRequest.contentRoot = contentRoot;
    importRequest.width = 4;
    importRequest.height = 4;

    AssetImportResult importResult;
    assert(importer.Import(importRequest, importResult));
    assert(importResult.Succeeded());
    assert(fs::exists(assetPath));

    TextureEditorAssetInfo textureInfo;
    assert(TextureImporter::ReadInfo(assetPath, textureInfo));
    assert(textureInfo.fileHeader.assetGuid == importResult.registryRecord.guid);
    assert(textureInfo.payloadHeader.width == 4);
    assert(textureInfo.payloadHeader.height == 4);
    assert(textureInfo.payloadHeader.sourceDataSize == fakePngBytes.size());
    assert(textureInfo.sourceExtension == ".png");

    AssetDatabase database(contentRoot);
    assert(database.Load());
    database.Registry().Upsert(importResult.registryRecord);

    MeshImporter meshImporter;
    AssetImportRequest meshImportRequest;
    meshImportRequest.sourcePath = meshSourcePath;
    meshImportRequest.destinationPath = meshAssetPath;
    meshImportRequest.contentRoot = contentRoot;

    AssetImportResult meshImportResult;
    assert(meshImporter.Import(meshImportRequest, meshImportResult));
    assert(meshImportResult.Succeeded());
    assert(fs::exists(meshAssetPath));

    MeshEditorAssetInfo meshInfo;
    assert(MeshImporter::ReadInfo(meshAssetPath, meshInfo));
    assert(meshInfo.fileHeader.assetGuid == meshImportResult.registryRecord.guid);
    assert(meshInfo.isCooked);
    assert(meshInfo.payloadHeader.version == kMeshAssetPayloadVersion);
    assert(meshInfo.sourceExtension == ".gltf");

    CookedMeshData cookedMesh;
    std::string cookedMeshError;
    assert(MeshImporter::ReadCookedData(meshAssetPath, cookedMesh, &cookedMeshError));
    assert(cookedMesh.vertices.size() == 3);
    assert(cookedMesh.indices.size() == 3);
    assert(cookedMesh.meshes.size() == 1);
    assert(cookedMesh.primitives.size() == 1);

    database.Registry().Upsert(meshImportResult.registryRecord);
    assert(database.Save());

    AssetDatabase loadedDatabase(contentRoot);
    assert(loadedDatabase.Load());
    const AssetRegistryRecord* byGuid = loadedDatabase.Registry().Find(importResult.registryRecord.guid);
    assert(byGuid != nullptr);
    assert(byGuid->type == AssetType::Texture);
    assert(byGuid->assetPath == "Assets/Textures/checker.textureasset");

    const AssetRegistryRecord* byPath =
        loadedDatabase.Registry().FindByAssetPath("Assets/Textures/checker.textureasset");
    assert(byPath != nullptr);
    assert(byPath->guid == importResult.registryRecord.guid);

    const AssetRegistryRecord* meshByPath =
        loadedDatabase.Registry().FindByAssetPath("Assets/Meshes/cube.meshasset");
    assert(meshByPath != nullptr);
    assert(meshByPath->guid == meshImportResult.registryRecord.guid);
    assert(meshByPath->type == AssetType::Mesh);

    std::error_code cleanupError;
    fs::remove_all(testRoot, cleanupError);

    std::cout << "Asset foundation tests passed: " << guidText << std::endl;
    return 0;
}

