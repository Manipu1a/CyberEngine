#pragma once
#include "platform/configure.h"
#include "math/basic_math.hpp"
#include "cyber_runtime.config.h"
#include "core/Core.h"
#include <EASTL/string.h>

CYBER_BEGIN_NAMESPACE(Cyber)
CYBER_BEGIN_NAMESPACE(Component)

    enum class ComponentType : uint8_t
    {
        Unknown = 0,
        Mesh,
        Camera,
        DirectionalLight,
    };

    // Base class for any component that carries a transform in world space.
    // Ownership: held by SceneNode via Scope<Primitive> (eastl::unique_ptr).
    // Polymorphism: virtual dtor + enum tag for RTTI-free dispatch, plus a
    // virtual clone() to support Duplicate in the scene hierarchy.
    class CYBER_RUNTIME_API Primitive
    {
    public:
        Primitive() = default;
        explicit Primitive(ComponentType t) : m_type(t) {}
        virtual ~Primitive() = default;

        ComponentType       type() const { return m_type; }
        virtual const char* type_name() const { return "Primitive"; }

        // Deep clone. Derived types copy their extra fields; runtime GPU
        // state (ModelLoader::Model, GPU buffers, gpu_ready) is reset so
        // the caller re-enqueues a load.
        virtual Scope<Primitive> clone() const = 0;

        float3       position{0.0f, 0.0f, 0.0f};
        quaternion_f rotation{0.0f, 0.0f, 0.0f, 1.0f};
        float3       scale{1.0f, 1.0f, 1.0f};
        bool         enabled = true;
        eastl::string name;

        float4x4 local_matrix() const
        {
            return float4x4::scale(scale) * rotation.to_matrix() * float4x4::translation(position);
        }

    protected:
        ComponentType m_type = ComponentType::Unknown;
    };

CYBER_END_NAMESPACE
CYBER_END_NAMESPACE
