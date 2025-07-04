#include "core/Timestep.h"

namespace Cyber
{
    void Timestep::restart()
    {
        start_time = std::chrono::high_resolution_clock::now();
    }

    double Timestep::get_seconds() const
    {
        auto duration = std::chrono::high_resolution_clock::now() - start_time;
        return std::chrono::duration_cast<std::chrono::duration<double>>(duration).count();
    }

    double Timestep::get_milliseconds() const
    {
        auto duration = std::chrono::high_resolution_clock::now() - start_time;
        return std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(duration).count(); //ms
    }
}