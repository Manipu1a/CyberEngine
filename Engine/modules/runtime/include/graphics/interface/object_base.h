#pragma once

#include "common/cyber_graphics_config.h"

namespace Cyber
{
    namespace RenderObject
    {
        template<class BaseInterface> 
        class CYBER_GRAPHICS_API ObjectBase : public BaseInterface
        {
        public:
            explicit ObjectBase() {};
            virtual ~ObjectBase() = default;
        };
    }
}