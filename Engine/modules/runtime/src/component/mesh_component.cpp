#include "component/mesh_component.h"
#include "editor/property_registry.h"

CYBER_REGISTER_COMPONENT(Cyber::Component::MeshComponent, "MeshComponent")
    .inherits("Primitive")
    .field("model_resource", &Cyber::Component::MeshComponent::model_resource).readonly();

CYBER_BEGIN_NAMESPACE(Cyber)
CYBER_BEGIN_NAMESPACE(Component)

    // Model ownership sits with the sample (see mesh_component.h). We leave
    // the raw pointer untouched so a separate cleanup pass can free it —
    // breaking the CyberRuntime <-> ModelLoader dep cycle.
    MeshComponent::~MeshComponent() = default;

    Scope<Primitive> MeshComponent::clone() const
    {
        auto copy = Scope<MeshComponent>(new MeshComponent());
        copy->position       = position;
        copy->rotation       = rotation;
        copy->scale          = scale;
        copy->enabled        = enabled;
        copy->name           = name;
        copy->model_resource = model_resource;
        // GPU-side state (mesh / model / gpu_ready) is intentionally left empty —
        // the caller enqueues a reload.
        return Scope<Primitive>(copy.release());
    }

CYBER_END_NAMESPACE
CYBER_END_NAMESPACE
