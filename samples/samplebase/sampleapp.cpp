#include "sampleapp.h"
#include "platform/memory.h"
#include "platform/windows/windows_application.h"

namespace Cyber
{
    namespace Samples
    {
        SampleApp::SampleApp()
        {
           // RHI::createRHI(ERHIBackend::RHI_BACKEND_D3D12);

        }
        SampleApp::~SampleApp()
        {

        }

        void SampleApp::initialize(Cyber::WindowDesc& desc)
        {
            Core::Application::create_application(desc);
            m_pApp = &Core::Application::getApp();
            m_pApp->initialize();
        }

        void SampleApp::run()
        {
        }

        void SampleApp::update(float deltaTime)
        {
        }
    }
}
