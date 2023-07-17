#pragma once
#include "gameruntime/GameApplication.h"

namespace Cyber
{
    namespace Samples
    {
        class TrignaleApp : public Cyber::GameApplication
        {
        public:
            TrignaleApp();
            ~TrignaleApp();

            virtual void initialize(Cyber::WindowDesc& desc);
            virtual void Run() override;
        };
    }
}
