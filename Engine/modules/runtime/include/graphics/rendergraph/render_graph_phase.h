#pragma once

namespace Cyber
{
    namespace render_graph
    {
        class RenderGraphPhase
        {
        public:
            RenderGraphPhase(const char8_t* name)
            {
                phase_name = name;
            }
            virtual ~RenderGraphPhase() = default;

            virtual void execute() = 0;
            class RenderGraphBuilder* builder;
            const char8_t* phase_name;
        };

        class RenderGraphPhase_Prepare : public RenderGraphPhase
        {
        public:
            RenderGraphPhase_Prepare() : RenderGraphPhase(u8"Prepare")
            {

            }
            virtual ~RenderGraphPhase_Prepare() = default;

            virtual void execute() override;
        };

        class RenderGraphPhase_Render : public RenderGraphPhase
        {
        public:
            RenderGraphPhase_Render() : RenderGraphPhase(u8"Render")
            {

            }
            virtual ~RenderGraphPhase_Render() = default;

            virtual void execute() override;
        };
    }
}