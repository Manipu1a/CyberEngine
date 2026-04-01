#pragma once
#include "cyber_game.config.h"
#include "platform/configure.h"
#include <eastl/vector.h>
#include "mesh.h"

CYBER_BEGIN_NAMESPACE(Cyber)
class CYBER_GAME_API World
{
public:
    World();

    void add_mesh(const RefCntAutoPtr<Mesh>& mesh)
    {
        meshes.push_back(mesh);
    }
protected:
    eastl::vector<RefCntAutoPtr<Mesh>> meshes;
};
CYBER_END_NAMESPACE