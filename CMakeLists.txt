option(BWEAS_BUILD_VCPKG "Build bweas with vcpkg" OFF)
if(WIN32 AND BWEAS_BUILD_VCPKG AND EXISTS "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")
    execute_process(COMMAND ${CMAKE_COMMAND} -E chdir vcpkg vcpkg/bootstrap-vcpkg.bat WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
    set(BWEAS_BUILD_VCPKG_N TRUE)
endif()

set(BW_NAME_PRJ bweas)

cmake_minimum_required(VERSION 3.20)
project(${BW_NAME_PRJ} LANGUAGES CXX C)

option(USER_BUILD_LZ4 "Rep LZ4 on external" OFF)
option(USER_BUILD_JSON "Rep JSON_C on external" OFF)
option(USER_BUILD_LUA "Rep LuaJit on external" OFF)
option(BWEAS_TEST "Build of bweas test" OFF)

set(CMAKE_CXX_STANDARD 17)
set(BWEAS_SLIB slbweas)

add_subdirectory(src/)
add_executable(${BW_NAME_PRJ} src/bweas.cpp)

target_link_libraries(${BW_NAME_PRJ} PRIVATE slbweas)

include_directories(${BWEAS_INCLUDE})

if(WIN32)
    file(COPY ${CMAKE_SOURCE_DIR}/bweas-config.json DESTINATION ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE})
else()
    file(COPY ${CMAKE_SOURCE_DIR}/bweas-config.json DESTINATION ${CMAKE_BINARY_DIR})

    install(TARGETS ${BW_NAME_PRJ} DESTINATION bin)
    install(TARGETS bwhook_safe DESTINATION bin)
    install(FILES ${CMAKE_SOURCE_DIR}/bweas-config.json DESTINATION bin)

    set_target_properties(bweas PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/bin")
endif()

if(BWEAS_TEST)
    enable_testing()
    add_subdirectory(test/)
endif()