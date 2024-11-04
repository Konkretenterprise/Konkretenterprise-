#include "pod5_format/thread_pool.h"

#include <boost/asio.hpp>

#include <optional>
#include <thread>

namespace pod5 {

class StrandImpl : public ThreadPoolStrand {
public:
    StrandImpl(boost::asio::io_context & context, std::shared_ptr<void> owner)
    : m_strand(context)
    , m_owner(owner)
    {
    }

    void post(std::function<void()> callback) override { boost::asio::post(m_strand, callback); }

    boost::asio::io_context::strand m_strand;
    std::shared_ptr<void> m_owner;
};

class ThreadPoolImpl : public ThreadPool, public std::enable_shared_from_this<ThreadPoolImpl> {
public:
    ThreadPoolImpl(std::size_t worker_count) : m_work(m_context)
    {
        assert(worker_count > 0);
        for (std::size_t i = 0; i < std::max<std::size_t>(1, worker_count); ++i) {
            m_threads.emplace_back([&] { m_context.run(); });
        }
    }

    ~ThreadPoolImpl()
    {
        stop_and_drain();
    }

    std::shared_ptr<ThreadPoolStrand> create_strand() override
    {
        if (m_stopped) {
            throw std::logic_error{"ThreadPool: create_strand() called after stop_and_drain()"};
        }
        return std::make_shared<StrandImpl>(m_context, shared_from_this());
    }

    void post(std::function<void()> callback) override {
        if (m_stopped) {
            throw std::logic_error{"ThreadPool: post() called after stop_and_drain()"};
        }
        boost::asio::post(m_context, std::move(callback));
    }

    void stop_and_drain() override {
        m_stopped = true;
        m_work.reset();
        for (auto & thread : m_threads) {
            if (thread.joinable()) {
                thread.join();
            }
        }
    }

    boost::asio::io_context m_context;
    std::optional<boost::asio::io_context::work> m_work;
    std::atomic<bool> m_stopped{false};
    std::vector<std::thread> m_threads;
};

std::shared_ptr<ThreadPool> make_thread_pool(std::size_t worker_threads)
{
    return std::make_shared<ThreadPoolImpl>(worker_threads);
}

}  // namespace pod5
