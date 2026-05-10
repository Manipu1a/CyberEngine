#pragma once
#include "cyber_runtime.config.h"
#include <thread>
#include <atomic>
#include <functional>

namespace Cyber
{
    namespace GameRuntime
    {
        class CYBER_RUNTIME_API AsyncLoader
        {
        public:
            enum class State
            {
                IDLE,
                RUNNING,
                COMPLETE,
                FAILED
            };

            AsyncLoader() = default;
            ~AsyncLoader();

            // Launch load_fn on a background thread
            void start(std::function<void()> load_fn);

            // Poll state (lock-free, safe from any thread)
            State get_state() const { return m_state.load(std::memory_order_acquire); }
            float get_progress() const { return m_progress.load(std::memory_order_relaxed); }
            const char* get_message() const { return m_message.load(std::memory_order_relaxed); }

            // Called from the loading function to report progress
            void set_progress(float progress, const char* message);

            // Block until the loading thread completes (for shutdown safety)
            void wait();

        private:
            std::atomic<State> m_state{State::IDLE};
            std::atomic<float> m_progress{0.0f};
            std::atomic<const char*> m_message{"Idle"};
            std::thread m_thread;
        };
    }
}
