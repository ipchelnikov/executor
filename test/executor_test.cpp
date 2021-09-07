#include "gtest/gtest.h"

#include "executor.h"

using namespace std::chrono_literals;

TEST(ExecutorTest, CreateExecutorWithoutArguments)
{
    Executor executor;
    EXPECT_EQ(executor.get_thead_count(), std::thread::hardware_concurrency());
}

TEST(ExecutorTest, CreateExecutorWithThreadCountArgument)
{
    auto thread_count = 10;
    Executor executor(thread_count);
    EXPECT_EQ(executor.get_thead_count(), thread_count);
}

TEST(ExecutorTest, DestroyExecutorWhileTaskIsRunning)
{
    std::atomic_bool is_terminating{false};
    std::atomic_bool is_compleated{false};
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
                is_compleated = true;
            });
            EXPECT_FALSE(is_terminating); // No termination befor distructor
        }};
    EXPECT_FALSE(is_compleated); // The function is running.

    while (!is_execution_started)
    {
        std::this_thread::sleep_for(2ms);
    }
    is_terminating = true; // Release the function.
    th.join();

    EXPECT_TRUE(is_compleated); // The function is completed.
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

    bool exception_is_trown = false;
    try
    {
        executor->push([] {});
    }
    catch (std::runtime_error e)
    {
        exception_is_trown = true;
        EXPECT_STREQ(e.what(), "Executor is halt");
    }

    EXPECT_TRUE(exception_is_trown);

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
    std::string random_exception{"Random exception"};

    bool exception_is_caught = false;
    {
        Executor executor(1,
                          [&](const Executor& ex, std::function<void()>,
                              const std::exception& e) {
                              auto runtime_exception =
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
