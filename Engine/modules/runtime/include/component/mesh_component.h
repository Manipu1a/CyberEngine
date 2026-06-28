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
        RefCntAutoPtr<RenderObject::IBuffer> vertex_buffer = nullptr;
        RefCntAutoPtr<RenderObject::IBuffer> index_buffer = nullptr;
        uint32_t                    vertex_stride = 0;
        eastl::vector<MeshDrawPrimitive> draw_primitives;
        bool                        gpu_ready = false;
    };

CYBER_END_NAMESPACE
CYBER_END_NAMESPACE
