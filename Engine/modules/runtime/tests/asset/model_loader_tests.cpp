#include "asset/mesh_importer.h"
#include "log/Log.h"
#include "model_loader.h"

#include <cassert>
#include <filesystem>
#include <iostream>

int main()
{
    namespace fs = std::filesystem;
    using namespace Cyber;

    Log::initLog();

    const fs::path mesh_asset =
        fs::current_path() / "Engine" / "content" / "Default" / "Cube.meshasset";
    assert(fs::exists(mesh_asset));

    MeshEditorAssetInfo asset_info;
    assert(MeshImporter::ReadInfo(mesh_asset, asset_info));
    assert(asset_info.sourceExtension == ".fbx");
    assert(asset_info.isCooked);

    ModelLoader::ModelCreateInfo create_info;
    const std::string source_path = mesh_asset.string();
    create_info.file_path = source_path.c_str();

    ModelLoader::Model model(create_info);
    model.load_data(create_info);
    assert(model.is_valid());
    assert(model.get_vertex_count() > 0);
    assert(model.get_index_count() > 0);
    assert(!model.get_meshes().empty());

    std::cout << "Cube.meshasset loaded: "
              << model.get_meshes().size() << " meshes, "
              << model.get_vertex_count() << " vertices, "
              << model.get_index_count() << " indices\n";

    return 0;
}
