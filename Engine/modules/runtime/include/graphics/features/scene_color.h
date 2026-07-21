#pragma once

#include "common/smart_ptr.h"
#include "graphics/features/forward_pass.h"

namespace Cyber::Renderer
{
    class CYBER_RUNTIME_API SceneColorPass final : public ForwardRenderPass
    {
    public:
        struct Resources
        {
            render_graph::RGTextureRef color = nullptr;
            render_graph::RGTextureRef depth = nullptr;
            render_graph::RGTextureRef shadow_map = nullptr;
        };

        SceneColorPass(const Resources& resources, ForwardPassContext* context);

        void setup(render_graph::RenderGraphBuilder& builder) override;
        void execute(render_graph::RenderGraph& graph, render_graph::RenderPassContext& context) override;

    private:
        void create_render_pass();

        Resources resources;
        RefCntAutoPtr<RenderObject::IRenderPass> render_pass;
    };
}
