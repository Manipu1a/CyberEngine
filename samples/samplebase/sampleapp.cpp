#include "sampleapp.h"

namespace Cyber
{
    namespace Samples
    {
        SampleApp::SampleApp()
        {
            immediate_context = RHI::createRHI(ERHIBackend::RHI_BACKEND_D3D12);
        }
        SampleApp::~SampleApp()
        {

        }

        void SampleApp::initialize(Cyber::WindowDesc& desc)
        {
        }

        void SampleApp::run()
        {
        }

        void SampleApp::update(float deltaTime)
        {
        }
    }
}
