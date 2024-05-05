#pragma once

class ImGuiRenderer
{
public:
    ImGuiRenderer();
    ~ImGuiRenderer();

    void new_frame();
    void end_frame();
    void render_draw_data(void* draw_data);
    void create_device_objects();
    void create_fonts_texture();
    
};