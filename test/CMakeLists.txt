project(bweas_test LANGUAGES CXX C)

file(GLOB_RECURSE SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/bs/*.cpp)

add_executable(${PROJECT_NAME} mtest.cpp ${SOURCE_FILES})

find_package(GTest CONFIG REQUIRED)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(${PROJECT_NAME} PRIVATE slbweas GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME})
