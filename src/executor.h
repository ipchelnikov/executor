#pragma once

#include <condition_variable>
#include <functional>
#include <thread>
#include <mutex>
#include <queue>

/**
 * @brief Task executor, allows to run functions (tasks) on an allocated thread pool.
 */
class Executor final
{
public:

    /**
     * Defines ExceptionHandler function type used in the constructor
     */
    typedef std::function<void(const Executor&,
                std::function<void()>, const std::exception&)> ExceptionHandler;

    /**
     * @brief Creates an executor with N threads.
     *
     * @param tread_count Desired number of threads. If it is 0 or not provided,
     * std::hardware_concurrency() value will be taken as the number of threads.
     * 
     * It might throw an error if hardware_concurrency() is 0, or
     * system error occurred while creating a threads.
     */
    explicit Executor(int thread_count = 0);

    /**
     * @brief Creates an executor with N threads and custom exception handler.
     *
     * @param tread_count Desired number of threads. If it is 0 or not provided,
     * std::hardware_concurrency() value will be taken as the number of threads.
     * @param handler The handler is triggered if an exception is occurred while
     * task is processing.
     *
     * It might throw an error if hardware_concurrency() is 0, or
     * system error occurred while creating a threads.
     */
    Executor(int thread_count, ExceptionHandler handler) :
            Executor(thread_count)
    {
        exception_handler = handler;
    }

    /**
     * @brief Terminates the executor gracefully. It waits for all scheduled
     * tasks to be finished then destroys the object.
     */
    ~Executor();

    // Copies are forbidden
    Executor(const Executor&) = delete;
    Executor& operator=(const Executor&) = delete;
    
    // Move constructors are disabled for the moment,
    // but they could be implemented if needed
    Executor(Executor&&) = delete;
    Executor& operator=(Executor&&) = delete;

    /**
     * @brief Pushes new task to the executor.
     * @param task Task to be scheduled.
     * 
     * The function throws runtime_error if pushing to a halt executor,
     * i.e. after executor destructor is triggered.
     */
    void push(std::function<void()> task);

    /**
     * @brief  Gets executor thread count.
     * @return Thread count.
     */
    int get_thead_count() { return workers.size(); }

    /**
     * @brief  Gets number of enqueued tasks.
     * @return Queue length.
     */
    int get_queue_length()
    {
        std::unique_lock<std::mutex> lk(queue_mutex);
        return task_queue.size();
    }

private:
    std::queue<std::function<void()>> task_queue;
    std::mutex queue_mutex;

    std::condition_variable condition;
    std::vector<std::thread> workers;

    bool halt = false;

    ExceptionHandler exception_handler =
        [](const Executor&, std::function<void()>, const std::exception&) {
            // Ignore exceptions by default
        };
};