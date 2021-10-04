#include "gtest/gtest.h"

#include "executor.h"

using namespace std::chrono_literals;

TEST(ExecutorTest, CreateExecutorWithoutArguments)
{
    const Executor executor;
    EXPECT_EQ(executor.get_thead_count(), std::thread::hardware_concurrency());
}

TEST(ExecutorTest, CreateExecutorWithThreadCountArgument)
{
    constexpr auto thread_count = 10;
    const Executor executor(thread_count);
    EXPECT_EQ(executor.get_thead_count(), thread_count);
}

TEST(ExecutorTest, DestroyExecutorWhileTaskIsRunning)
{
    std::atomic_bool is_terminating{false};
    std::atomic_bool is_completed{false};
    std::atomic_bool is_execution_started{false};

    std::thread th{
        [&] {
            Executor executor(1);
            executor.push([&] {
                while (!is_terminating)
                {
                    is_execution_started = true;
                    std::this_thread::sleep_for(10ms);
                }
                is_completed = true;
            });
            EXPECT_FALSE(is_terminating); // No termination before destructor
        }};
    EXPECT_FALSE(is_completed); // The function is running.

    while (!is_execution_started)
    {
        std::this_thread::sleep_for(2ms);
    }
    is_terminating = true; // Release the function.
    th.join();

    EXPECT_TRUE(is_completed); // The function is completed.
}

TEST(ExecutorTest, PushTaskToHaltExecutor)
{
    std::atomic_bool is_terminating{false};
    std::atomic_bool is_execution_started{false};

    Executor *executor = nullptr;

    std::thread th{
        [&] {
            executor = new Executor{1};
            executor->push([&] {
                while (!is_terminating)
                {
                    is_execution_started = true;
                    std::this_thread::sleep_for(10ms);
                }
            });
            delete executor;
            executor = nullptr;
        }};

    while (!is_execution_started)
    {
        std::this_thread::sleep_for(2ms);
    }

    bool exception_is_thrown = false;
    try
    {
        executor->push([] {});
    }
    catch (std::runtime_error &e)
    {
        exception_is_thrown = true;
        EXPECT_STREQ(e.what(), "Executor is halt");
    }

    EXPECT_TRUE(exception_is_thrown);

    is_terminating = true;
    th.join();
}

TEST(ExecutorTest, PushTaskWhichThrowsExceptionWithDefaultHandler)
{
    Executor executor(1);
    executor.push([] { throw std::runtime_error("Random exception"); });

    // Nothing to expect, just should not terminate the program
}

TEST(ExecutorTest, PushTaskWhichThrowsExceptionWithCustomHandler)
{
    const std::string random_exception{"Random exception"};

    bool exception_is_caught = false;
    {
        Executor executor(1,
                          [&](const Executor& ex, std::function<void()>,
                              const std::exception& e) {
                              const auto runtime_exception =
                                  dynamic_cast<const std::runtime_error*>(&e);

                              EXPECT_NE(runtime_exception, nullptr);

                              EXPECT_STREQ(
                                  random_exception.c_str(),
                                  runtime_exception->what());

                              exception_is_caught = true;
                          });
        executor.push([&] { throw std::runtime_error(random_exception); });
    }

    EXPECT_TRUE(exception_is_caught);
}

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
