add_executable(example example.cpp)
add_dependencies(example executor)

target_include_directories(example PUBLIC ../src)
target_link_libraries(example executor pthread)
