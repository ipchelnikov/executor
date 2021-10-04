#include <chrono>
#include <iostream>
#include <atomic>

#include "executor.h"

using namespace std::chrono_literals;

int main()  // NOLINT(bugprone-exception-escape)
{
    std::cout << "Welcome to the executor example.\n";

    std::atomic_size_t tasks_left;

    {
        Executor executor; // Create executor with the default thread count

        const auto executor_count = executor.get_thead_count();

        tasks_left = executor_count * 2;

        std::cout << "Executor is created with "
                  << executor_count << " treads pool.\n";

        std::cout << "Running " << executor_count * 2
                  << " tasks on the executor.\n";

        for (size_t i = 0; i < executor_count * 2; ++i)
        {
            executor.push([&tasks_left] {
                std::this_thread::sleep_for(100ms);
                --tasks_left;
            });
        }

        std::cout << "Queue length after scheduling all the tasks is: "
                  << executor.get_queue_length() << ".\n";
    }
    std::cout << "Executor is terminated.\n";

    if (tasks_left == 0)
    {
        std::cout << "All tasks are successfully finished.\n";
    }

    return 0;
}
