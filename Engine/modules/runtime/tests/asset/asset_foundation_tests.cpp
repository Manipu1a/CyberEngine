#include "asset/asset.h"

#include <cassert>
#include <cstring>
#include <iostream>

int main()
{
    using namespace Cyber;

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

    std::cout << "Asset foundation tests passed: " << guidText << std::endl;
    return 0;
}

