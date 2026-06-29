#pragma once
#include "gameruntime/sampleapp.h"
#include "gameruntime/scene_node.h"
#include "gameruntime/cyber_game.config.h"
#include "common/smart_ptr.h"
#include "EASTL/vector.h"

namespace Cyber
{
    namespace ModelLoader
    {
        class Model;
    }

    namespace Component
    {
        class MeshComponent;
    }

    namespace Samples
    {
        class CYBER_GAME_API SponzaApp : public SampleApp
        {
            struct SponzaVertex
            {
                float3 position;
                float3 normal;
                float2 uv;
            };

            struct SceneConstants
            {
                float4x4 view_proj_matrix;
                float4x4 model_matrix;
                float4 camera_pos;
                float4 light_direction;
                float4 light_color;
            };

        public:
            SponzaApp();
            ~SponzaApp();

            virtual void update(float deltaTime) override;
            virtual void draw_ui(ImGuiContext* in_imgui_context) override;

            virtual void on_create_gfx_objects() override;
            virtual void on_create_pipelines() override;
            virtual void on_load_data() override;
            virtual void on_create_resources() override;

        protected:
            bool build_render_mesh_for_component(SceneNode& node, Component::MeshComponent& mc);
            void process_pending_loads();

            // Ensure there's always a camera + sun we can drive. Creates
            // defaults during on_load_data when the scene file didn't include
            // one (pre-v2 scene without a camera/sun node, etc.).
            void ensure_camera_and_sun();

            float camera_orbit_speed = 0.0f;
        };
    }
}
