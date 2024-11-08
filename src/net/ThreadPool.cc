#include "ThreadPool.h"

using namespace inet;

ThreadPool::ThreadPool() :
    m_running(false)
{}

ThreadPool::~ThreadPool()
{
    if(m_running) {
        stop();
    }
}

void ThreadPool::start(int numThreads)
{
    m_running = true;
    m_threads.reserve(numThreads);

    for (int i = 0; i < numThreads; i++) {
        m_threads.emplace_back(std::make_unique<Thread>([this]() {
            runInThread();
        }));

        m_threads[i]->start();
    }
}

void ThreadPool::stop()
{
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_running = false;
        m_cond.notify_all();
    }

    for (auto &thread : m_threads) {
        thread->join();
    }
}

void ThreadPool::add(Task task)
{
	if (m_threads.empty()) {	//若没有线程，就直接执行任务
		task();
	} else {
		{
			std::unique_lock<std::mutex> lock(m_mutex);
			if (!m_running)
				return;
			m_tasks.push(std::move(task));
		}
		m_cond.notify_one();
	}   
}

void ThreadPool::runInThread()
{
    while (m_running) {
        Task task;
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_cond.wait(lock, [this]() { 
                return !m_running || !m_tasks.empty();
            });

            if (!m_tasks.empty()) {
                task = std::move(m_tasks.front());
                m_tasks.pop();
            }
        }

        if (task) {
            task();
        }
    }
}