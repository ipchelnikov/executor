file(GLOB SRC_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

add_library(executor STATIC ${SRC_FILES})

target_link_libraries(executor) #pthread

target_include_directories(executor PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})