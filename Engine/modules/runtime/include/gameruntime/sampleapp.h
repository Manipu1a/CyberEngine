#pragma once
#include "cyber_game.config.h"
#include "application/application.h"
#include "gameruntime/world.h"
#include "graphics/interface/graphics_types.h"
#include "resource/resource_loader.h"

namespace Cyber
{
    namespace Renderer
    {
        class Renderer;
    }

    namespace RenderObject
    {
        class IRenderDevice;
        class IDeviceContext;
        class ISwapChain;
        class IFrameBuffer;
        class IRenderPass;
        class IRenderPipeline;
        class IRootSignature;
        class IDescriptorSet;
        class IShaderLibrary;
        class ISampler;
        class IBuffer;
        class ITexture;
        class ITexture_View;
    }

    namespace Samples
    {
        struct ShaderDesc
        {
            const char8_t* path;
            SHADER_STAGE stage;
            const char8_t* entry_point;
            SHADER_TARGET target = SHADER_TARGET_6_0;
        };

        struct FrameContext
        {
            RenderObject::IRenderDevice* render_device = nullptr;
            RenderObject::IDeviceContext* device_context = nullptr;
            RenderObject::ISwapChain* swap_chain = nullptr;
            RenderObject::IFrameBuffer* frame_buffer = nullptr;
            RenderObject::ITexture* color_buffer = nullptr;
            RenderObject::ITexture* depth_buffer = nullptr;
            RenderObject::ITexture_View* color_view = nullptr;
            RenderObject::ITexture_View* depth_view = nullptr;
        };

        class CYBER_GAME_API SampleApp
        {
        public:
            SampleApp();
            virtual ~SampleApp();

            // --- Core lifecycle (called by engine) ---
            virtual void initialize();
            virtual void run();
            virtual void update(float deltaTime);
            virtual void present();
            virtual void draw_ui(ImGuiContext* in_imgui_context) {}

            // --- Initialization hooks (override in subclass) ---
            virtual void on_create_gfx_objects() {}
            virtual void on_create_pipelines() {}
            virtual void on_create_resources() {}

            // --- Accessors ---
            uint32_t get_back_buffer_index() const { return m_backBufferIndex; }
            RefCntAutoPtr<World> get_world() const { return m_world; }

            static SampleApp* create_sample_app();

        protected:
            // --- Convenience accessors ---
            Renderer::Renderer* get_renderer() const;
            RenderObject::IRenderDevice* get_render_device() const;
            RenderObject::IDeviceContext* get_device_context() const;

            // --- Frame rendering helpers ---
            // Acquires back buffer, sets barriers, begins render pass, sets viewport/scissor
            FrameContext begin_frame(
                RenderObject::IRenderPass* render_pass,
                const GRAPHICS_CLEAR_VALUE& clear_color = {0.690196097f, 0.768627524f, 0.870588303f, 1.000000000f});

            // Ends render pass (does NOT call cmd_end/flush/present - that's in present())
            void end_frame();

            // --- Resource creation helpers ---
            RefCntAutoPtr<RenderObject::IShaderLibrary> load_shader(const ShaderDesc& desc);

            void create_buffer(
                GRAPHICS_RESOURCE_BIND_FLAGS bind_flags,
                uint32_t size,
                GRAPHICS_RESOURCE_USAGE usage,
                CPU_ACCESS_FLAGS cpu_access,
                const void* initial_data,
                RefCntAutoPtr<RenderObject::IBuffer>& out_buffer);

            void create_vertex_buffer(const void* data, uint32_t size, RefCntAutoPtr<RenderObject::IBuffer>& out_buffer);
            void create_index_buffer(const void* data, uint32_t size, RefCntAutoPtr<RenderObject::IBuffer>& out_buffer);
            void create_constant_buffer(uint32_t size, RefCntAutoPtr<RenderObject::IBuffer>& out_buffer);

            // Map/write/unmap in one call
            void update_buffer(RenderObject::IBuffer* buffer, const void* data, uint32_t size);

            template<typename T>
            void update_buffer(RenderObject::IBuffer* buffer, const T& data)
            {
                update_buffer(buffer, &data, sizeof(T));
            }

            // Create a standard single-subpass render pass (color + depth)
            void create_default_render_pass(const char8_t* name, RefCntAutoPtr<RenderObject::IRenderPass>& out_render_pass);

            // Create a default linear wrap sampler
            RefCntAutoPtr<RenderObject::ISampler> create_default_sampler();

        protected:
            uint32_t m_backBufferIndex = 0;
            Core::Application* m_pApp = nullptr;
            RefCntAutoPtr<World> m_world;
        };
    }
}
