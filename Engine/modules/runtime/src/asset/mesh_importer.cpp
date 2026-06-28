#include "asset/mesh_importer.h"

#include "asset/asset_hash.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <fstream>
#include <limits>

namespace Cyber
{
    namespace
    {
        std::string lowercase(std::string_view text)
        {
            std::string out(text);
            std::transform(out.begin(), out.end(), out.begin(),
                [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            return out;
        }

        bool write_mesh_editor_asset(const AssetImportRequest& request,
                                     const std::vector<uint8_t>& sourceBytes,
                                     AssetGuid assetGuid,
                                     AssetFileHeader& outHeader)
        {
            const std::string sourceExtension = lowercase(request.sourcePath.extension().string());
            if (sourceExtension.size() > std::numeric_limits<uint32_t>::max())
                return false;

            MeshAssetPayloadHeader payloadHeader;
            payloadHeader.sourceExtensionSize = static_cast<uint32_t>(sourceExtension.size());
            payloadHeader.sourceDataOffset = sizeof(MeshAssetPayloadHeader) + sourceExtension.size();
            payloadHeader.sourceDataSize = sourceBytes.size();

            AssetFileHeader fileHeader;
            fileHeader.assetType = AssetType::Mesh;
            fileHeader.assetGuid = assetGuid.IsValid() ? assetGuid : AssetGuid::Create();
            fileHeader.contentHash = AssetHash::HashBytes(sourceBytes.data(), sourceBytes.size());
            fileHeader.dependencyHash = AssetHash::HashString(request.sourcePath.generic_string());
            fileHeader.cookerVersion = kMeshImporterVersion;
            fileHeader.platformTag = kAnyAssetPlatform;
            fileHeader.payloadOffset = sizeof(AssetFileHeader);
            fileHeader.payloadSize = payloadHeader.sourceDataOffset + payloadHeader.sourceDataSize;

            std::error_code ec;
            const std::filesystem::path parent = request.destinationPath.parent_path();
            if (!parent.empty())
                std::filesystem::create_directories(parent, ec);
            if (ec)
                return false;

            std::ofstream file(request.destinationPath, std::ios::binary | std::ios::trunc);
            if (!file)
                return false;

            file.write(reinterpret_cast<const char*>(&fileHeader), sizeof(fileHeader));
            file.write(reinterpret_cast<const char*>(&payloadHeader), sizeof(payloadHeader));
            file.write(sourceExtension.data(), static_cast<std::streamsize>(sourceExtension.size()));
            if (!sourceBytes.empty())
            {
                file.write(reinterpret_cast<const char*>(sourceBytes.data()),
                           static_cast<std::streamsize>(sourceBytes.size()));
            }

            if (!file.good())
                return false;

            outHeader = fileHeader;
            return true;
        }
    }

    bool MeshImporter::Import(const AssetImportRequest& request,
                              AssetImportResult& outResult) const
    {
        outResult = {};

        if (request.sourcePath.empty() || request.destinationPath.empty())
        {
            outResult.error = "Mesh import requires source and destination paths.";
            return false;
        }

        if (!IsSupportedSourceExtension(request.sourcePath.extension().string()))
        {
            outResult.error = "Unsupported mesh source extension.";
            return false;
        }

        std::vector<uint8_t> sourceBytes;
        if (!ReadSourceBytes(request.sourcePath, sourceBytes))
        {
            outResult.error = "Failed to read mesh source file.";
            return false;
        }

        AssetFileHeader fileHeader;
        if (!write_mesh_editor_asset(request, sourceBytes, request.existingGuid, fileHeader))
        {
            outResult.error = "Failed to write mesh editor asset.";
            return false;
        }

        AssetRegistryRecord record;
        record.guid = fileHeader.assetGuid;
        record.type = AssetType::Mesh;
        record.assetPath = AssetRegistry::MakeStoredPath(request.destinationPath, request.contentRoot);
        record.displayName = request.destinationPath.stem().string();
        record.sourcePath = AssetRegistry::MakeStoredPath(request.sourcePath, request.contentRoot);
        record.sourceHash = fileHeader.contentHash;
        record.editorAssetHash = AssetHash::Combine(fileHeader.contentHash, fileHeader.dependencyHash);
        record.importerVersion = Version();
        record.assetFormatVersion = kAssetFileFormatVersion;

        outResult.registryRecord = std::move(record);
        return true;
    }

    bool MeshImporter::IsSupportedSourceExtension(std::string_view extension)
    {
        const std::string ext = lowercase(extension);
        static constexpr std::array<std::string_view, 3> kSupportedExtensions {{
            ".fbx", ".gltf", ".glb"
        }};

        return std::find(kSupportedExtensions.begin(), kSupportedExtensions.end(), ext) !=
               kSupportedExtensions.end();
    }

    bool MeshImporter::ReadInfo(const std::filesystem::path& path,
                                MeshEditorAssetInfo& outInfo)
    {
        outInfo = {};

        std::ifstream file(path, std::ios::binary);
        if (!file)
            return false;

        AssetFileHeader fileHeader;
        file.read(reinterpret_cast<char*>(&fileHeader), sizeof(fileHeader));
        if (!file || !fileHeader.IsCompatible(AssetType::Mesh, kAssetFileFormatVersion))
            return false;

        if (fileHeader.payloadOffset < sizeof(AssetFileHeader) ||
            fileHeader.payloadSize < sizeof(MeshAssetPayloadHeader))
        {
            return false;
        }

        file.seekg(static_cast<std::streamoff>(fileHeader.payloadOffset), std::ios::beg);
        MeshAssetPayloadHeader payloadHeader;
        file.read(reinterpret_cast<char*>(&payloadHeader), sizeof(payloadHeader));
        if (!file ||
            payloadHeader.magic != kMeshAssetPayloadMagic ||
            payloadHeader.version != kMeshAssetPayloadVersion ||
            payloadHeader.sourceExtensionSize > 64)
        {
            return false;
        }

        const uint64_t minSourceOffset =
            sizeof(MeshAssetPayloadHeader) + payloadHeader.sourceExtensionSize;
        if (payloadHeader.sourceDataOffset < minSourceOffset ||
            payloadHeader.sourceDataOffset > fileHeader.payloadSize)
        {
            return false;
        }

        std::string sourceExtension;
        sourceExtension.resize(payloadHeader.sourceExtensionSize);
        if (!sourceExtension.empty())
            file.read(sourceExtension.data(), static_cast<std::streamsize>(sourceExtension.size()));
        if (!file)
            return false;

        outInfo.fileHeader = fileHeader;
        outInfo.payloadHeader = payloadHeader;
        outInfo.sourceExtension = std::move(sourceExtension);
        return true;
    }

    bool MeshImporter::ReadSourceBytes(const std::filesystem::path& path,
                                       std::vector<uint8_t>& outBytes)
    {
        outBytes.clear();

        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (!file)
            return false;

        const std::streamoff size = file.tellg();
        if (size < 0)
            return false;

        outBytes.resize(static_cast<size_t>(size));
        file.seekg(0, std::ios::beg);
        if (!outBytes.empty())
            file.read(reinterpret_cast<char*>(outBytes.data()), static_cast<std::streamsize>(outBytes.size()));
        return file.good() || file.eof();
    }
}
