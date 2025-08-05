#pragma once
#include "gameruntime/sampleapp.h"
#include "model_loader.h"
#include "component/camera_component.h"
#include "gameruntime/cyber_game.config.h"

namespace Cyber
{
    namespace GUI
    {
        class GUIApplication;
    }
    
    namespace RenderObject
    {
        class IRootSignature;
        class IDescriptorSet;
        class IRenderPipeline;
    }

    namespace Samples
    {
        class CYBER_GAME_API PBRApp : public SampleApp
        {
            struct CubeVertex
            {
                float3 position;
                float3 normal;
                float3 tangent;
                float2 uv;
            };

            struct ConstantMatrix
            {
                float4x4 ModelMatrix;
                float4x4 ViewMatrix;
                float4x4 ProjectionMatrix;
            };

            struct PrecomputeEnvMapAttribs
            {
                float4x4 rotation;

                float roughness;
                float env_map_size;
                uint32_t num_samples;
                float dummy;
            };

            struct MaterialResourceBinding
            {
                ModelLoader::Material::MaterialAttribs attribs;
                RenderObject::IRenderPipeline* model_pipeline = nullptr;

                RenderObject::ITexture_View* base_color_texture_view = nullptr;
                RenderObject::ITexture_View* metallic_roughness_texture_view = nullptr;
                RenderObject::ITexture_View* normal_texture_view = nullptr;
                RenderObject::ITexture_View* occlusion_texture_view = nullptr;
                RenderObject::ITexture_View* emissive_texture_view = nullptr;
            };

            struct ModelResourceBinding
            {
                ModelLoader::Model* model = nullptr;
                
                RenderObject::IBuffer* vertex_buffer = nullptr;
                RenderObject::IBuffer* index_buffer = nullptr;
                RenderObject::IBuffer* vertex_constant_buffer = nullptr;
                
                float4x4 model_transform = float4x4::Identity();

                eastl::vector<MaterialResourceBinding> material_bindings;

                ModelResourceBinding() {}
            };

        public:
            PBRApp();
            ~PBRApp();

            virtual void initialize() override;
            virtual void run() override;
            virtual void update(float deltaTime) override;
            virtual void present() override;

        public:
            void raster_draw();
            void create_gfx_objects();
            void create_render_pipeline();
            void create_resource();
            void create_ui();
            virtual void draw_ui(ImGuiContext* in_imgui_context) override;
            void finalize();

        protected:
            void precompute_environment_map();
            void bind_material_resources(RenderObject::IDeviceContext* device_context, const MaterialResourceBinding& material_binding);
            eastl::vector<ShaderMacro> get_shader_macros(const MaterialResourceBinding& material_binding) const;

        protected:
            RenderObject::IDescriptorSet* descriptor_set = nullptr;
            RenderObject::IRenderPipeline* environment_pipeline = nullptr;
            RenderObject::IRenderPipeline* irradiance_pipeline = nullptr;
            RenderObject::IRenderPipeline* prefiltered_pipeline = nullptr;

            RenderObject::RenderSubpassDesc subpass_desc[2];
            RenderObject::RenderPassAttachmentDesc attachment_desc;

            RenderObject::IRenderPass* render_pass = nullptr;

            eastl::vector<ModelResourceBinding> model_resource_bindings;

            RenderObject::ITexture_View* environment_texture_view = nullptr;
            RenderObject::ITexture_View* prefiltered_cube_texture_view = nullptr;
            RenderObject::ITexture* irradiance_cube_texture = nullptr;

            static constexpr uint32_t irradiance_cube_size = 64;
            static constexpr uint32_t prefiltered_cube_size = 256;

            RenderObject::IBuffer* light_constant_buffer = nullptr;
            RenderObject::IBuffer* camera_constant_buffer = nullptr;
            RenderObject::IBuffer* precompute_env_map_buffer = nullptr;

            float3 light_direction = { 0.0f, 0.0f, -10.0f };
            float3 light_color = { 1.0f, 1.0f, 1.0f };

            float3 model_position = { 0.0f, 0.0f, 0.0f };
            float3 model_scale = { 1.0f, 1.0f, 1.0f };
            float3 model_rotation = { 0.0f, 0.0f, 0.0f };
            float3 camera_position = { 0.0f, 0.0f, -30.0f };
            
            Component::CameraComponent* camera_component = nullptr;

        };
    }

}
