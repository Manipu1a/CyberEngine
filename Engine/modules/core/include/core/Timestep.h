#pragma once
#include <chrono>
#include "cyber_core.config.h"

namespace Cyber
{
    class CYBER_CORE_API Timestep
    {
    public:
        Timestep()
        {
            restart();
        }

        void restart();
        double get_seconds() const;
        double get_milliseconds() const;

    private:
        std::chrono::high_resolution_clock::time_point start_time;
    };
}