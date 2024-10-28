cmake_minimum_required(VERSION 3.20)

set(BW_NAME_PRJ bweas)

option(USER_BUILD_LZ4 "Rep LZ4 on external" OFF)
option(USER_BUILD_JSON "Rep JSON_C on external" OFF)
option(USER_BUILD_LUA "Rep LuaJit on external" OFF)

set(CMAKE_CXX_STANDARD 17)
project(${BW_NAME_PRJ} LANGUAGES CXX C)

file(GLOB KERNEL_LL "src/kernel/low_level/*.c")
file(GLOB KERNEL_HL "src/kernel/high_level/assistant.cpp")
if(WIN32)
    file(GLOB KERNEL_HL_SAFE_DLL "src/kernel/high_level/hook_winapi.cpp")
endif()
file(GLOB KERNEL ${KERNEL_LL} ${KERNEL_HL} ${KERNEL_HL_SAFE_DLL})

file(GLOB HAX "src/bs/console_arg/hax.cpp")
file(GLOB LANG "src/bs/lang/*.hpp" "src/bs/lang/*.cpp")
file(GLOB MODULE "src/bs/smodule/*.hpp" "src/bs/smodule/*.cpp" "src/bs/smodule/srl/*.hpp" "src/bs/smodule/srl/*.cpp")
file(GLOB BS "src/bs/*.cpp" ${LANG})
file(GLOB MAIN "src/main.cpp" "src/mdef.cpp")
file(GLOB FILE_SRC ${MAIN} ${KERNEL} ${BS} ${HAX} ${MODULE})

add_executable(${BW_NAME_PRJ} ${FILE_SRC})

# LuaJit
if(USER_BUILD_LUA)
    set(CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/external/")
    find_package(LuaJIT REQUIRED)
    message(STATUS "Selected LuaJit in folder - external")
else()
    find_library(LuaJIT_LIBRARIES NAMES libluajit-5.1.so)
    if(NOT LuaJIT_LIBRARIES)
        message(FATAL_ERROR "LuaJIT not found!")
    endif()

    set(LuaJIT_SRC_DIR = "/usr/include/luajit-2.1")
endif()
target_link_libraries(${BW_NAME_PRJ} PRIVATE ${LuaJIT_LIBRARIES})
include_directories(${LuaJIT_SRC_DIR})

# nlohmann_json
if(USER_BUILD_JSON)
    add_subdirectory(external/json)
    target_link_libraries(${BW_NAME_PRJ} PRIVATE nlohmann_json::nlohmann_json)

    message(STATUS "Selected nlohmann_json in folder - external")
else()
    find_package(nlohmann_json REQUIRED)
    target_link_libraries(${BW_NAME_PRJ} PRIVATE nlohmann_json::nlohmann_json)
    include_directories(${NLOHMANN_JSON_INCLUDE_INSTALL_DIR})
endif()

# LZ4
if(USER_BUILD_LZ4)
    set(LZ4_BUILD_CLI OFF)
    add_subdirectory(external/lz4/build/cmake)
    message(STATUS "Selected LZ4 in folder - external")
else()
    find_library(LZ4_LIBRARY NAMES lz4)
endif()
target_link_libraries(${BW_NAME_PRJ} PRIVATE lz4)

if(WIN32)
    if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
        message(STATUS "Release build")
    endif()
    
    message(STATUS "Enabling the detours lib(Microsoft)")
    if (${CMAKE_SIZEOF_VOID_P} STREQUAL 4)
        message(STATUS "Selected lib 32 bit build")
        target_link_libraries(${BW_NAME_PRJ} PUBLIC ${CMAKE_SOURCE_DIR}/external/detours/lib.X86/detours.lib)
    elseif (${CMAKE_SIZEOF_VOID_P} STREQUAL 8)
        message(STATUS "Selected lib 64 bit build")
        target_link_libraries(${BW_NAME_PRJ} PUBLIC ${CMAKE_SOURCE_DIR}/external/detours/lib.X64/detours.lib)
    endif ()
    include_directories("external/detours/include")

    file(COPY ${LuaJIT_SRC_DIR}/${LuaJIT_NAME_LIBRARIES}.dll DESTINATION ${CMAKE_BINARY_DIR}/Debug)
else()
    message(STATUS "Internal enabled interception library for securely connecting modules and plugins on a UNIX system")
    add_subdirectory(src/kernel/high_level/hook_linux_lib)
    target_link_libraries(${BW_NAME_PRJ} PUBLIC bwhook_safe)

    file(COPY ${LuaJIT_LIBRARIES} DESTINATION ${CMAKE_BINARY_DIR})
endif()
