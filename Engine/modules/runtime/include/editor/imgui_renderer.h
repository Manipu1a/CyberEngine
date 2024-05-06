#pragma once

namespace Cyber
{
    namespace Editor
    {
        class ImGuiRenderer
        {
        public:
            ImGuiRenderer();
            ~ImGuiRenderer();

            void new_frame();
            void end_frame();
            void render_draw_data();
            void invalidate_device_objects();
            void create_device_objects();
            void create_fonts_texture();
        };
    }
}
