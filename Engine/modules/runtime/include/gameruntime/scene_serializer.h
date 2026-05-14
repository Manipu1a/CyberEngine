#pragma once
#include "cyber_game.config.h"
#include "world.h"

namespace Cyber
{
    namespace SceneSerializer
    {
        // `path` is a project-relative path (e.g.
        // "samples/sponza/assets/sponza.scene") or an absolute path. The
        // function resolves it against the project root when relative.
        //
        // Writes the World as v2 JSON: each node emits a `components` array
        // of Primitive-derived components (Mesh/Camera/DirectionalLight).
        // MeshComponent model_resource paths are normalized to project-relative.
        //
        // Returns false on write failure. On success, World::is_dirty() is
        // cleared and World::source_path() is updated to `path`.
        CYBER_GAME_API bool save(World& world, const char* path);

        // Loads a scene file into `world` — existing contents are cleared.
        // Handles both v2 (components array) and v1 (legacy transform +
        // mesh.source + root camera/sun) shapes; v1 is migrated to v2
        // in-memory. MeshComponents are populated with model_resource,
        // but GPU resource creation still happens in the sample's
        // on_load_data / on_create_resources.
        //
        // Returns false on parse or IO failure.
        CYBER_GAME_API bool load(World& world, const char* path);
    }
}
