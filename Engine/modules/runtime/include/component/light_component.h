#pragma once 
#include "platform/configure.h"
#include "math/basic_math.hpp"
#include "cyber_runtime.config.h"

CYBER_BEGIN_NAMESPACE(Cyber)
CYBER_BEGIN_NAMESPACE(Component)

struct CYBER_RUNTIME_API LightAttribs
{
    float4 light_direction;
    float4 light_intensity;
    float4 camera_position;
};

struct CYBER_RUNTIME_API LightComponent
{
    
};

CYBER_END_NAMESPACE
CYBER_END_NAMESPACE
