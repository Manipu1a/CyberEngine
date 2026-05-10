#include "gameruntime/async_loader.h"
#include "log/log.h"

namespace Cyber
{
    namespace GameRuntime
    {
        AsyncLoader::~AsyncLoader()
        {
            wait();
        }

        void AsyncLoader::start(std::function<void()> load_fn)
        {
            wait(); // ensure any previous thread is joined

            m_state.store(State::RUNNING, std::memory_order_release);
            m_progress.store(0.0f, std::memory_order_relaxed);
            m_message.store("Loading...", std::memory_order_relaxed);

            m_thread = std::thread([this, fn = std::move(load_fn)]()
            {
                try
                {
                    fn();
                    m_state.store(State::COMPLETE, std::memory_order_release);
                }
                catch (const std::exception& e)
                {
                    CB_CORE_ERROR("AsyncLoader: exception during loading: {0}", e.what());
                    m_state.store(State::FAILED, std::memory_order_release);
                }
                catch (...)
                {
                    CB_CORE_ERROR("AsyncLoader: unknown exception during loading");
                    m_state.store(State::FAILED, std::memory_order_release);
                }
            });
        }

        void AsyncLoader::set_progress(float progress, const char* message)
        {
            m_progress.store(progress, std::memory_order_relaxed);
            if (message)
                m_message.store(message, std::memory_order_relaxed);
        }

        void AsyncLoader::wait()
        {
            if (m_thread.joinable())
                m_thread.join();
        }
    }
}
