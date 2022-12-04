#pragma once

#include "cyber_runtime.config.h"

namespace Cyber
{
    class CYBER_RUNTIME_API Timestep
    {
    public:
        Timestep(float time = 0.f)
            : mTime(time)
        {

        }

        operator float() const {return mTime;}

        float GetSeconds() const {return mTime;}
        float GetMilliseconds() const { return mTime * 1000.0f; }
    private:
        float mTime;
    };
}