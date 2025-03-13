#ifndef INET_THREADPOOL_H
#define INET_THREADPOOL_H

#include <functional>
#include <mutex>
#include <vector>
#include <queue>

#include "Thread.h"

namespace inet
{
    class ThreadPool
    {
    public:
        using Task = std::function<void()>;

        explicit ThreadPool();
        ~ThreadPool();

        void start(int numThreads);
        void stop();

        void add(Task task);

    private:
        void runInThread();

        std::mutex m_mutex;
        std::condition_variable m_cond;
        std::vector<std::unique_ptr<Thread>> m_threads;
        std::queue<Task> m_tasks;
        bool m_running;
    };

} // namespace inet

#endif
