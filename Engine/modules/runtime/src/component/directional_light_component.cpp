#include "component/directional_light_component.h"
#include "editor/property_registry.h"
#include <cmath>

CYBER_REGISTER_COMPONENT(Cyber::Component::DirectionalLightComponent, "DirectionalLightComponent")
    .inherits("Primitive")
    .field("color",     &Cyber::Component::DirectionalLightComponent::color).as_color()
    .field("intensity", &Cyber::Component::DirectionalLightComponent::intensity).min(0.0f).max(1000.0f).speed(0.05f);

CYBER_BEGIN_NAMESPACE(Cyber)
CYBER_BEGIN_NAMESPACE(Component)

namespace
{
    constexpr float3 kDefaultDirection(0.0f, -1.0f, 0.0f);
}

Scope<Primitive> DirectionalLightComponent::clone() const
{
    auto copy = Scope<DirectionalLightComponent>(new DirectionalLightComponent());
    copy->position  = position;
    copy->rotation  = rotation;
    copy->scale     = scale;
    copy->enabled   = enabled;
    copy->name      = name;
    copy->color     = color;
    copy->intensity = intensity;
    return Scope<Primitive>(copy.release());
}

float3 DirectionalLightComponent::get_direction() const
{
    float3 d = rotation.rotate(kDefaultDirection);
    return Math::normalize(d);
}

void DirectionalLightComponent::set_direction(const float3& dir_in)
{
    float3 target = Math::normalize(dir_in);
    float cos_theta = Math::dot(kDefaultDirection, target);

    // Nearly same direction — identity rotation.
    if (cos_theta > 0.999999f)
    {
        rotation = quaternion_f(0.0f, 0.0f, 0.0f, 1.0f);
        return;
    }

    // Nearly opposite — 180° around any axis perpendicular to the default.
    if (cos_theta < -0.999999f)
    {
        rotation = quaternion_f::rotation_from_axis_angle(float3(1.0f, 0.0f, 0.0f), 3.14159265358979323846f);
        return;
    }

    float3 axis  = Math::cross(kDefaultDirection, target);
    float  angle = std::acos(cos_theta);
    rotation = quaternion_f::rotation_from_axis_angle(axis, angle);
}

CYBER_END_NAMESPACE
CYBER_END_NAMESPACE
