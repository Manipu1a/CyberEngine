#pragma once
#include "component/primitive.h"
#include "gameruntime/mesh.h"
#include "common/smart_ptr.h"
#include <EASTL/string.h>

CYBER_BEGIN_NAMESPACE(Cyber)

namespace ModelLoader { class Model; }

CYBER_BEGIN_NAMESPACE(Component)

    class CYBER_RUNTIME_API MeshComponent : public Primitive
    {
    public:
        MeshComponent() : Primitive(ComponentType::Mesh) {}
        ~MeshComponent() override;

        const char*      type_name() const override { return "MeshComponent"; }
        Scope<Primitive> clone() const override;

        // Project-relative path to a model asset (.gltf/.glb). Serialized.
        eastl::string               model_resource;

        // Runtime state — populated by the sample once GPU resources exist.
        // Not serialized.
        RefCntAutoPtr<Mesh>         mesh;
        // ModelLoader::Model is owned by the sample (ModelLoader lives in a
        // module that depends on CyberRuntime — we can't call its destructor
        // from here without a dep cycle). The sample cleans up in its own
        // dtor by walking components.
        ModelLoader::Model*         model = nullptr;
        bool                        gpu_ready = false;
    };

CYBER_END_NAMESPACE
CYBER_END_NAMESPACE
