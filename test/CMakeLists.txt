#########################################
# Fetch GoogleTest
include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        e2239ee6043f73722e7aa812a459f54a28552929 # release-1.11.0
)
FetchContent_MakeAvailable(googletest)

#########################################
# Configure the test to use GoogleTest
add_executable(executor_test executor_test.cpp)
add_dependencies(executor_test executor)

target_include_directories(executor_test PUBLIC ${googletest_SOURCE_DIR}/googletest/include)
target_include_directories(executor_test PUBLIC ../src)
target_link_libraries(executor_test executor gtest gtest_main pthread)

#########################################
# Make the test runnable with $ make check
enable_testing()
add_test( NAME ExecutorTest COMMAND executor_test )
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND}
                  DEPENDS executor_test)