#include "asset/mesh_importer.h"
#include "log/Log.h"

#include <filesystem>
#include <fstream>
#include <iostream>

int main(int argc, char** argv)
{
    namespace fs = std::filesystem;
    using namespace Cyber;

    Log::initLog();
    if (argc >= 3)
    {
        const fs::path sourcePath = fs::path(argv[1]);
        const fs::path destinationPath = fs::path(argv[2]);
        const fs::path contentRoot = argc >= 4 ? fs::path(argv[3]) : destinationPath.parent_path();

        AssetImportRequest request;
        request.sourcePath = sourcePath;
        request.contentRoot = contentRoot;
        MeshEditorAssetInfo existingInfo;
        if (MeshImporter::ReadInfo(destinationPath, existingInfo))
            request.existingGuid = existingInfo.fileHeader.assetGuid;

        const fs::path temporaryRoot = fs::current_path() / "Saved" / "MeshAssetMigration";
        std::error_code ec;
        fs::create_directories(temporaryRoot, ec);
        if (ec)
            return 1;
        request.destinationPath = temporaryRoot / (destinationPath.filename().string() + ".cooking");

        MeshImporter importer;
        AssetImportResult result;
        if (!importer.Import(request, result))
        {
            std::cerr << "Cook failed: " << result.error << '\n';
            return 1;
        }
        fs::copy_file(request.destinationPath, destinationPath,
                      fs::copy_options::overwrite_existing, ec);
        if (ec)
        {
            std::cerr << "Failed to replace destination: " << ec.message() << '\n';
            return 1;
        }
        CookedMeshData cooked;
        std::string error;
        if (!MeshImporter::ReadCookedData(destinationPath, cooked, &error))
        {
            std::cerr << "Cooked asset verification failed: " << error << '\n';
            return 1;
        }
        fs::remove_all(temporaryRoot, ec);
        std::cout << "Cooked " << destinationPath << ": "
                  << cooked.meshes.size() << " meshes, "
                  << cooked.vertices.size() << " vertices, "
                  << cooked.indices.size() << " indices\n";
        return 0;
    }

    const fs::path assetPath = argc > 1
        ? fs::path(argv[1])
        : fs::current_path() / "Engine" / "content" / "Default" / "Cube.meshasset";

    MeshEditorAssetInfo legacyInfo;
    if (!MeshImporter::ReadInfo(assetPath, legacyInfo))
    {
        std::cerr << "Invalid mesh asset: " << assetPath << '\n';
        return 1;
    }
    if (legacyInfo.isCooked)
    {
        std::cout << "Already cooked: " << assetPath << '\n';
        return 0;
    }

    std::vector<uint8_t> sourceBytes;
    if (!MeshImporter::ReadEmbeddedSource(assetPath, legacyInfo, sourceBytes))
    {
        std::cerr << "Legacy source payload is unreadable: " << assetPath << '\n';
        return 1;
    }

    const fs::path temporaryRoot = fs::current_path() / "Saved" / "MeshAssetMigration";
    std::error_code ec;
    fs::create_directories(temporaryRoot, ec);
    if (ec)
        return 1;

    const fs::path sourcePath = temporaryRoot /
        (legacyInfo.fileHeader.assetGuid.ToString() + legacyInfo.sourceExtension);
    const fs::path cookedPath = temporaryRoot /
        (legacyInfo.fileHeader.assetGuid.ToString() + ".meshasset");
    {
        std::ofstream source(sourcePath, std::ios::binary | std::ios::trunc);
        source.write(reinterpret_cast<const char*>(sourceBytes.data()),
                     static_cast<std::streamsize>(sourceBytes.size()));
        if (!source.good())
            return 1;
    }

    AssetImportRequest request;
    request.sourcePath = sourcePath;
    request.destinationPath = cookedPath;
    request.contentRoot = assetPath.parent_path();
    request.existingGuid = legacyInfo.fileHeader.assetGuid;

    MeshImporter importer;
    AssetImportResult result;
    if (!importer.Import(request, result))
    {
        std::cerr << "Migration cook failed: " << result.error << '\n';
        return 1;
    }

    fs::copy_file(cookedPath, assetPath, fs::copy_options::overwrite_existing, ec);
    if (ec)
    {
        std::cerr << "Failed to replace legacy asset: " << ec.message() << '\n';
        return 1;
    }

    CookedMeshData cooked;
    std::string readError;
    if (!MeshImporter::ReadCookedData(assetPath, cooked, &readError))
    {
        std::cerr << "Migrated asset verification failed: " << readError << '\n';
        return 1;
    }

    fs::remove_all(temporaryRoot, ec);
    std::cout << "Migrated " << assetPath << ": "
              << cooked.meshes.size() << " meshes, "
              << cooked.vertices.size() << " vertices, "
              << cooked.indices.size() << " indices\n";
    return 0;
}
