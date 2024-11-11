project(bweas_slib LANGUAGES CXX C)

file(GLOB_RECURSE FILE_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
list(REMOVE_ITEM FILE_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/bweas.cpp)
if(NOT WIN32)
    list(REMOVE_ITEM FILE_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/kernel/high_level/hook_winapi.cpp)
endif()

add_library(${BWEAS_SLIB} STATIC ${FILE_SOURCE})

# Build with vcpkg
if(BWEAS_BUILD_VCPKG_N)
  set(LuaJIT_LIBRARIES "${CMAKE_SOURCE_DIR}/vcpkg/packages/luajit_x64-windows/lib/lua51.lib")
  set(LuaJIT_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/vcpkg/packages/luajit_x64-windows/include/luajit")

  find_package(nlohmann_json CONFIG REQUIRED)
  find_package(lz4 CONFIG REQUIRED)

  set(BWEAS_LIBS ${LuaJIT_LIBRARIES} nlohmann_json::nlohmann_json lz4::lz4)
  set(BWEAS_INCLUDE ${LuaJIT_INCLUDE_DIRS} CACHE STRING "All header-files for bweas")

  message(STATUS "Install package with vcpkg")
else()
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

        set(LuaJIT_SRC_DIR "/usr/include/luajit-2.1" "/usr/local/include/luajit-2.1")
    endif()

    # nlohmann_json
    if(USER_BUILD_JSON)
        add_subdirectory(external/json)

        message(STATUS "Selected nlohmann_json in folder - external")
    else()
        find_package(nlohmann_json REQUIRED)
    endif()

    # LZ4
    if(USER_BUILD_LZ4)
        set(LZ4_BUILD_CLI OFF)
        add_subdirectory(external/lz4/build/cmake)
        message(STATUS "Selected LZ4 in folder - external")
    else()
        find_library(LZ4_LIBRARY NAMES lz4)
    endif()

    set(BWEAS_LIBS ${LuaJIT_LIBRARIES} nlohmann_json::nlohmann_json lz4)
    set(BWEAS_INCLUDE ${NLOHMANN_JSON_INCLUDE_INSTALL_DIR} ${LuaJIT_SRC_DIR} CACHE STRING "All header-files for bweas")
endif()

target_link_libraries(${BWEAS_SLIB} PRIVATE ${BWEAS_LIBS})
include_directories(${BWEAS_INCLUDE})

if(WIN32)
    if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
        message(STATUS "Release build")
    endif()
    
    message(STATUS "Enabling the detours lib(Microsoft)")
    if (${CMAKE_SIZEOF_VOID_P} STREQUAL 4)
        message(STATUS "Selected lib 32 bit build")
        target_link_libraries(${BWEAS_SLIB} PRIVATE ${CMAKE_SOURCE_DIR}/external/detours/lib.X86/detours.lib)
    elseif (${CMAKE_SIZEOF_VOID_P} STREQUAL 8)
        message(STATUS "Selected lib 64 bit build")
        target_link_libraries(${BWEAS_SLIB} PRIVATE ${CMAKE_SOURCE_DIR}/external/detours/lib.X64/detours.lib)
    endif ()
    include_directories("../external/detours/include")
else()
    message(STATUS "Internal enabled interception library for securely connecting modules and plugins on a UNIX system")
    add_subdirectory(kernel/high_level/hook_linux_lib)
    target_link_libraries(${BWEAS_SLIB} PRIVATE bwhook_safe)
endif()