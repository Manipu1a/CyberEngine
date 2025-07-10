#pragma once 
#include "platform/configure.h"
#include "math/basic_math.hpp"
#include "cyber_runtime.config.h"

CYBER_BEGIN_NAMESPACE(Cyber)
CYBER_BEGIN_NAMESPACE(Component)

struct CYBER_RUNTIME_API CameraAttribs
{
    float4 camera_position;
    //float4x4 view_matrix;
    //float4x4 projection_matrix;
    //float4x4 view_projection_matrix;
    float4x4 inverse_view_projection_matrix;
};

class CYBER_RUNTIME_API CameraComponent
{
    
};

CYBER_END_NAMESPACE
CYBER_END_NAMESPACE
