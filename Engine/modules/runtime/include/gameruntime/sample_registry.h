#pragma once
#include "cyber_game.config.h"
#include "sampleapp.h"

namespace Cyber
{
    namespace Samples
    {
        struct SampleEntry
        {
            const wchar_t* display_name;
            SampleApp* (*factory)();
        };

        CYBER_GAME_API SampleApp* create_triangle_app();
        CYBER_GAME_API SampleApp* create_cube_app();
        CYBER_GAME_API SampleApp* create_pbrdemo_app();
        CYBER_GAME_API SampleApp* create_shadow_app();
        CYBER_GAME_API SampleApp* create_sponza_app();

        const SampleEntry* sample_registry();
        int sample_registry_count();
    }
}
