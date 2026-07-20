#include "component/mesh_component.h"

CYBER_BEGIN_NAMESPACE(Cyber)
CYBER_BEGIN_NAMESPACE(Component)

    // Model ownership is type-erased to keep CyberRuntime independent of ModelLoader.
    MeshComponent::~MeshComponent()
    {
        release_runtime_model();
    }

    void MeshComponent::set_runtime_model(ModelLoader::Model* in_model, ModelDeleter deleter)
    {
        release_runtime_model();
        model = in_model;
        model_deleter = deleter;
    }

    void MeshComponent::release_runtime_model()
    {
        if (model && model_deleter)
            model_deleter(model);
        model = nullptr;
        model_deleter = nullptr;
    }

    Scope<Primitive> MeshComponent::clone() const
    {
        auto copy = Scope<MeshComponent>(new MeshComponent());
        copy->position       = position;
        copy->rotation       = rotation;
        copy->scale          = scale;
        copy->enabled        = enabled;
        copy->name           = name;
        copy->model_resource = model_resource;
        // GPU-side state (mesh / model / gpu_ready) is intentionally left empty;
        // the caller enqueues a reload.
        return Scope<Primitive>(copy.release());
    }

CYBER_END_NAMESPACE
CYBER_END_NAMESPACE
