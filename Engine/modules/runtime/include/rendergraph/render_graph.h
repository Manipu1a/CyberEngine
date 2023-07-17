#pragma once

#include "render_graph_flag.h"
#include "render_graph_builder.h"

class RenderGraph
{
public:
    //static ERenderGraphResult Create();

    void Destroy();

    void Update();

private:
    RenderGraph() = default;
    ~RenderGraph() = default;

    class RenderGraphBuilder* graphBuilder;
};