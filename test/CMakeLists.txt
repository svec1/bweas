set(CMAKE_CXX_STANDARD 17)
project(bweas_test LANGUAGES CXX C)

add_executable(${PROJECT_NAME} mtest.cpp)

find_package(GTest CONFIG REQUIRED)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(${PROJECT_NAME} PRIVATE slbweas GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME})