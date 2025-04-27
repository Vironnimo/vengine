#pragma once

#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <queue>
#include <atomic>
#include <future>
#include <memory>
#include <spdlog/spdlog.h>

namespace Vengine {

enum class TaskPriority { Low, Normal, High, Critical };

struct Task {
    std::function<void()> function;
    TaskPriority priority = TaskPriority::Normal;
    std::string name;

    // order tasks by priority
    bool operator<(const Task& other) const {
        return priority < other.priority;
    }
};

class ThreadManager {
   public:
    ThreadManager(size_t threadCount = 0) {
        if (threadCount == 0) {
            // -1 to leave one thread for the main thread
            threadCount = std::max<size_t>(1, std::thread::hardware_concurrency() - 1);
        }

        spdlog::debug("Constructor ThreadManager, {} worker threads", threadCount);
        startWorkers(threadCount);
    }

    ~ThreadManager() {
        shutdown();
    }

    template <typename F>
    auto enqueueTask(F&& func, const std::string& name = "",
                     TaskPriority priority = TaskPriority::Normal) -> std::future<decltype(func())> {
        using ReturnType = decltype(func());

        auto task = std::make_shared<std::packaged_task<ReturnType()>>(std::forward<F>(func));
        std::future<ReturnType> result = task->get_future();

        Task wrappedTask;
        wrappedTask.name = name;
        wrappedTask.priority = priority;
        wrappedTask.function = [task]() { (*task)(); };

        {
            std::lock_guard<std::mutex> lock(m_queueMutex);
            m_tasks.push(std::move(wrappedTask));
        }
        m_condition.notify_one();

        return result;
    }

    template <typename F>
    void enqueueMainThreadTask(F&& func, const std::string& name = "") {
        Task task;
        task.function = std::forward<F>(func);
        task.name = name;
        task.priority = TaskPriority::Normal;  

        std::lock_guard<std::mutex> lock(m_mainThreadMutex);
        m_mainThreadTasks.push(std::move(task));
    }

    void processMainThreadTasks() {
        std::queue<Task> tasks;
        {
            std::lock_guard<std::mutex> lock(m_mainThreadMutex);
            tasks.swap(m_mainThreadTasks);
        }

        while (!tasks.empty()) {
            Task& task = tasks.front();

            try {
                // spdlog::debug("Main thread executing task: {}", task.name);
                task.function();
            } catch (const std::exception& e) {
                spdlog::error("Exception in main thread task '{}': {}", task.name, e.what());
            }

            tasks.pop();
        }
    }

    void waitForCompletion() {
        bool tasksInQueue = false;
        {
            std::lock_guard<std::mutex> lock(m_queueMutex);
            tasksInQueue = !m_tasks.empty();
        }

        if (!tasksInQueue) {
            std::unique_lock<std::mutex> lock(m_busyMutex);
            m_completionCondition.wait(lock, [this] { return m_busyCount == 0; });
        } else {
            while (true) {
                {
                    std::unique_lock<std::mutex> lock(m_busyMutex);
                    if (m_busyCount == 0) {
                        std::lock_guard<std::mutex> queueLock(m_queueMutex);
                        if (m_tasks.empty()) {
                            break;
                        }
                    } else {
                        m_completionCondition.wait_for(lock, std::chrono::milliseconds(10));
                    }
                }

                // small sleep 
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }
    }

    void shutdown() {
        {
            std::lock_guard<std::mutex> lock(m_queueMutex);
            m_shutdown = true;
        }

        m_condition.notify_all();

        for (auto& worker : m_workers) {
            if (worker.joinable()) {
                worker.join();
            }
        }
        m_workers.clear();

        spdlog::info("ThreadManager shutdown complete");
    }

   private:
    void startWorkers(size_t threadCount) {
        for (size_t i = 0; i < threadCount; ++i) {
            m_workers.emplace_back([this, i] {
                // spdlog::debug("Worker thread {} started", i);

                while (true) {
                    Task task;

                    {
                        std::unique_lock<std::mutex> lock(m_queueMutex);
                        m_condition.wait(lock, [this] { return !m_tasks.empty() || m_shutdown; });

                        if (m_shutdown && m_tasks.empty()) {
                            break;
                        }

                        if (!m_tasks.empty()) {
                            task = m_tasks.top();
                            m_tasks.pop();
                        }
                    }

                    if (task.function) {
                        {
                            std::lock_guard<std::mutex> lock(m_busyMutex);
                            ++m_busyCount;
                        }

                        try {
                            // spdlog::debug("Thread {} executing task: {}", i, task.name);
                            task.function();
                        } catch (const std::exception& e) {
                            spdlog::error("Exception in task '{}': {}", task.name, e.what());
                        }

                        {
                            std::lock_guard<std::mutex> lock(m_busyMutex);
                            --m_busyCount;
                        }
                        m_completionCondition.notify_all();
                    }
                }

                spdlog::debug("Worker thread {} exiting", i);
            });
        }
    }

    std::vector<std::thread> m_workers;
    std::priority_queue<Task> m_tasks;
    std::mutex m_queueMutex;
    std::mutex m_busyMutex;
    std::condition_variable m_condition;
    std::condition_variable m_completionCondition;
    std::atomic<bool> m_shutdown{false};
    std::atomic<size_t> m_busyCount{0};

    std::queue<Task> m_mainThreadTasks;
    std::mutex m_mainThreadMutex;
};

}  // namespace Vengine