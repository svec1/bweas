option(BWEAS_BUILD_VCPKG "Build bweas with vcpkg" OFF)
if(WIN32 AND BWEAS_BUILD_VCPKG AND EXISTS "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")
    execute_process(COMMAND ${CMAKE_COMMAND} -E chdir vcpkg vcpkg/bootstrap-vcpkg.bat WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
    set(BWEAS_BUILD_VCPKG_N TRUE)
endif()

cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

option(USER_BUILD_LZ4 "Rep LZ4 on external" OFF)
option(USER_BUILD_JSON "Rep JSON_C on external" OFF)
option(USER_BUILD_LUA "Rep LuaJit on external" OFF)
option(BWEAS_TEST "Build of bweas test" OFF)

set(CMAKE_CXX_STANDARD 23)

set(BW_NAME_PRJ bweas)
set(BWEAS_SLIB slbweas)

project(${BW_NAME_PRJ} LANGUAGES CXX)

add_subdirectory(src/)
add_executable(${BW_NAME_PRJ} src/bweas.cpp)

target_link_libraries(${BW_NAME_PRJ} PRIVATE slbweas)

include_directories(${BWEAS_INCLUDE} "src/bs/")

if(WIN32)
    file(COPY ${CMAKE_SOURCE_DIR}/bweas-config.json DESTINATION ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE})
else()
    file(COPY ${CMAKE_SOURCE_DIR}/bweas-config.json DESTINATION ${CMAKE_BINARY_DIR})
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/packages)

    file(GLOB SOURCE_FILES_PACKAGES ${CMAKE_SOURCE_DIR}/packages/*.json)
    foreach(SOURCE_FILE IN LISTS SOURCE_FILES_PACKAGES)
        file(READ ${SOURCE_FILE} JSON_CONFIG_PACKAGE)
        string(JSON NAME_PACKAGE GET ${JSON_CONFIG_PACKAGE} "package-name")
        list(APPEND BWEAS_PACKAGES_OUTPUT "${NAME_PACKAGE}.bweas-package")
    endforeach()

    message(STATUS ${BWEAS_PACKAGES_OUTPUT})

    add_custom_target(
        BWEAS_PACKAGES ALL 
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR} 
        DEPENDS ${BWEAS_PACKAGES_OUTPUT}
        VERBATIM
    )

    set(I_SOURCE_FILE 0)
    foreach(SOURCE_FILE IN LISTS SOURCE_FILES_PACKAGES)
        list(GET BWEAS_PACKAGES_OUTPUT ${I_SOURCE_FILE} NAME_PACKAGE)
        add_custom_command(
            OUTPUT ${BWEAS_PACKAGES_OUTPUT}
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/packages
            COMMAND bweas --nolog --package ${SOURCE_FILE}
            DEPENDS ${SOURCE_FILE} bweas
            COMMENT "Compiled bweas package ${BWEAS_PACKAGES_OUTPUT}"
        )
        set(I_SOURCE_FILE ${I_SOURCE_FILE}+1)
    endforeach()

    add_dependencies(BWEAS_PACKAGES ${BW_NAME_PRJ})

    install(TARGETS ${BW_NAME_PRJ} DESTINATION bin)
    install(FILES ${CMAKE_SOURCE_DIR}/bweas-config.json DESTINATION bin)
    install(DIRECTORY ${CMAKE_BINARY_DIR}/packages DESTINATION bin)

    set_target_properties(bweas PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/bin")
endif()

if(BWEAS_TEST)
    enable_testing()
    add_subdirectory(test/)
endif()
