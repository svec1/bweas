set(CMAKE_CXX_STANDARD 17)
project(bweas_test LANGUAGES CXX C)

add_executable(${PROJECT_NAME} mtest.cpp)

find_package(GTest CONFIG REQUIRED)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/src ${BWEAS_INCLUDE})
target_link_libraries(${PROJECT_NAME} PUBLIC GTest::gtest_main ${BWEAS_LIBS})

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME})