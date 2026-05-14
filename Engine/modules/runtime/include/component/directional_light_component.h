#pragma once
#include "component/primitive.h"
#include "math/basic_math.hpp"
#include "cyber_runtime.config.h"

CYBER_BEGIN_NAMESPACE(Cyber)
CYBER_BEGIN_NAMESPACE(Component)

// Directional light as a scene component. The light's direction is derived
// from Primitive::rotation applied to the default down vector (0,-1,0), so
// moving the rotation in the editor visually reorients the light.
class CYBER_RUNTIME_API DirectionalLightComponent : public Primitive
{
public:
    DirectionalLightComponent() : Primitive(ComponentType::DirectionalLight) {}
    ~DirectionalLightComponent() override = default;

    const char*      type_name() const override { return "DirectionalLightComponent"; }
    Scope<Primitive> clone() const override;

    // Compute the normalized direction vector in world space by rotating
    // the default down axis (0,-1,0) by the component's rotation.
    float3 get_direction() const;

    // Set rotation such that the direction vector matches `dir` (up to
    // the choice of roll). Handy when migrating old scene data that stored
    // a direction rather than a quaternion.
    void set_direction(const float3& dir);

    // Serialized.
    float3 color{1.0f, 1.0f, 1.0f};
    float  intensity = 3.0f;
};

CYBER_END_NAMESPACE
CYBER_END_NAMESPACE
