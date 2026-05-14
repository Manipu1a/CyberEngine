#include "component/primitive.h"
#include "editor/property_registry.h"

// Register the transform-carrying base class with the property registry.
// Derived components chain `.inherits("Primitive")` so these fields are
// drawn in the Details panel automatically.
CYBER_REGISTER_COMPONENT(Cyber::Component::Primitive, "Primitive")
    .field("name",     &Cyber::Component::Primitive::name)
    .field("enabled",  &Cyber::Component::Primitive::enabled)
    .field("position", &Cyber::Component::Primitive::position).speed(0.05f)
    .field("rotation", &Cyber::Component::Primitive::rotation).speed(0.5f)
    .field("scale",    &Cyber::Component::Primitive::scale   ).speed(0.01f).min(0.0001f).uniform_scale();
