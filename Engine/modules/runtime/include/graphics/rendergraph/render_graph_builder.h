#pragma once
#include "render_graph_resource.h"
#include "EASTL/map.h"
#include "EASTL/vector.h"
#include "platform/configure.h"
#include "platform/memory.h"
#include "interface/graphics_types.h"

namespace Cyber
{
    namespace RenderObject
    {
        struct IRenderDevice;
        struct IQueue;
    }
    
    namespace render_graph
    {
        class CYBER_RUNTIME_API RenderGraphBuilder
        {
        public:
            explicit RenderGraphBuilder(class RenderGraph* owner_graph = nullptr);
            ~RenderGraphBuilder();
            RenderGraphBuilder& backend_api(GRAPHICS_BACKEND backend) CYBER_NOEXCEPT;
            RenderGraphBuilder& with_device(class RenderObject::IRenderDevice* device) CYBER_NOEXCEPT;
            RenderGraphBuilder& with_queue(class RenderObject::IQueue* queue) CYBER_NOEXCEPT;
            
        public:
            RGTextureRef create_texture(RGTextureCreateDesc desc, const char8_t* name);
            RGTextureRef import_texture(RenderObject::ITexture* texture, const char8_t* name);
            void update_imported_texture(RGTextureRef texture, RenderObject::ITexture* imported_texture);
            RGTextureRef get_texture(const char8_t* name);

            RenderObject::ITexture* GetRHITexture(const char8_t* name);
            
            RGBufferRef create_buffer(RGBufferCreateDesc desc, const char8_t* name);
            RGBufferRef get_buffer(const char8_t* name);

            RGTextureViewRef CreateTextureView(RGTextureViewCreateDesc desc);

            void add_render_pass(const char8_t* name, const render_pass_function& func, const render_pass_execute_function& execute_func);

            template<typename PassType, typename... Args>
            PassType& add_pass(const char8_t* name, Args&&... args)
            {
                PassType* pass = cyber_new<PassType>(eastl::forward<Args>(args)...);
                register_pass(name, pass, &destroy_typed_pass<PassType>);
                pass->setup(*this);
                return *pass;
            }

            void add_compute_pass();

        private:
            void register_pass(const char8_t* name, RGPass* pass, PassNode::PassDestroyFunction destroy_pass);

            template<typename PassType>
            static void destroy_typed_pass(RGPass* pass)
            {
                cyber_delete(static_cast<PassType*>(pass));
            }

        public:
            class RenderObject::IRenderDevice* device = nullptr;
            class RenderObject::IQueue* gfx_queue = nullptr;
            class RenderGraph* graph = nullptr;
        };
    }

}
