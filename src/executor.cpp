#include "executor.h"

Executor::Executor(unsigned int thread_count)
{
    if (!thread_count)
    {
        thread_count = std::thread::hardware_concurrency();
    }
    if (!thread_count)
    {
        throw std::runtime_error("Cannot determine thread count.");
    }

    for (unsigned int i = 0; i < thread_count; ++i)
    {
        workers.emplace_back([this] {
            std::function<void()> task;

            while (true)
            {
                {
                    std::unique_lock<std::mutex> lk(queue_mutex);
                    condition.wait(lk, [this] { return halt || !task_queue.empty(); });

                    if (halt && task_queue.empty())
                    {
                        return;
                    }

                    task = std::move(task_queue.front());
                    task_queue.pop();
                }
                try
                {
                    task();
                }
                catch (std::exception& e)
                {
                    exception_handler(*this, std::move(task), e);
                }
            }
        });
    }
}

Executor::~Executor()
{
    {
        std::unique_lock<std::mutex> lk(queue_mutex);
        halt = true;
    }
    condition.notify_all();

    for (auto &tread : workers)
    {
        tread.join();
    }
}

void Executor::push(std::function<void()> task)
{
    if (task == nullptr)
    {
        throw(std::runtime_error("Invalid function"));
    }

    {
        std::unique_lock<std::mutex> lk(queue_mutex);

        if (halt)
        {
            throw(std::runtime_error("Executor is halt"));
        }

        task_queue.emplace(std::move(task));
    }
    condition.notify_one();
}