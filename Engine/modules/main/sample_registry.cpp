#include "gameruntime/sample_registry.h"

namespace Cyber
{
    namespace Samples
    {
        static const SampleEntry g_entries[] = {
#ifdef CYBER_HAS_TRIANGLE
            { L"Triangle", &create_triangle_app },
#endif
#ifdef CYBER_HAS_CUBE
            { L"Cube", &create_cube_app },
#endif
#ifdef CYBER_HAS_PBRDEMO
            { L"PBR Demo", &create_pbrdemo_app },
#endif
#ifdef CYBER_HAS_SHADOW
            { L"Shadow", &create_shadow_app },
#endif
#ifdef CYBER_HAS_SPONZA
            { L"Sponza", &create_sponza_app },
#endif
            { nullptr, nullptr }
        };

        const SampleEntry* sample_registry()
        {
            return g_entries;
        }

        int sample_registry_count()
        {
            int n = 0;
            while (g_entries[n].display_name != nullptr) ++n;
            return n;
        }
    }
}
