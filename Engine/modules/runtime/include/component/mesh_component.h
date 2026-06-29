#pragma once
#include "component/primitive.h"
#include "gameruntime/mesh.h"
#include "common/smart_ptr.h"
#include "graphics/interface/graphics_types.h"
#include <EASTL/string.h>
#include <EASTL/vector.h>

CYBER_BEGIN_NAMESPACE(Cyber)

namespace ModelLoader { class Model; }
namespace RenderObject
{
    struct IBuffer;
    struct ITexture_View;
}

CYBER_BEGIN_NAMESPACE(Component)

    struct MeshDrawPrimitive
    {
        uint32_t first_index = 0;
        uint32_t index_count = 0;
        RefCntAutoPtr<RenderObject::ITexture_View> base_color_view = nullptr;
    };

    class CYBER_RUNTIME_API MeshComponent : public Primitive
    {
    public:
        using ModelDeleter = void (*)(ModelLoader::Model*);

        MeshComponent() : Primitive(ComponentType::Mesh) {}
        ~MeshComponent() override;

        const char*      type_name() const override { return "MeshComponent"; }
        Scope<Primitive> clone() const override;
        void set_runtime_model(ModelLoader::Model* in_model, ModelDeleter deleter);
        void release_runtime_model();

        // Project-relative path to a model asset (.gltf/.glb). Serialized.
        eastl::string               model_resource;

        // Runtime state — populated by the sample once GPU resources exist.
        // Not serialized.
        RefCntAutoPtr<Mesh>         mesh;
        // ModelLoader::Model is released through model_deleter to avoid a
        // CyberRuntime -> ModelLoader dependency.
        ModelLoader::Model*         model = nullptr;
        ModelDeleter                model_deleter = nullptr;
        RefCntAutoPtr<RenderObject::IBuffer> vertex_buffer = nullptr;
        RefCntAutoPtr<RenderObject::IBuffer> index_buffer = nullptr;
        uint32_t                    vertex_stride = 0;
        eastl::vector<MeshDrawPrimitive> draw_primitives;
        bool                        gpu_ready = false;
    };

CYBER_END_NAMESPACE
CYBER_END_NAMESPACE
