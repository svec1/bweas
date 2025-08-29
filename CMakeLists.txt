option(USER_BUILD_LZ4 "Rep LZ4 on external" OFF)
option(USER_BUILD_JSON "Rep JSON_C on external" OFF)
option(USER_BUILD_LUA "Rep LuaJit on external" OFF)
option(BWEAS_TEST "Build of bweas test" OFF)

cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_STANDARD 23)

set(BW_NAME_PRJ bweas)
set(BWEAS_SLIB slbweas)

project(${BW_NAME_PRJ} LANGUAGES CXX)

add_subdirectory(src/)
add_executable(${BW_NAME_PRJ} src/bweas.cpp)

target_link_libraries(${BW_NAME_PRJ} PRIVATE slbweas)

include_directories(${BWEAS_INCLUDE} "src/bs/")

if(BWEAS_TEST)
    enable_testing()
    add_subdirectory(test/)
endif()

if(WIN32)
    set(CURRENT_BINARY_DIR ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE})
else()
    set(CURRENT_BINARY_DIR ${CMAKE_BINARY_DIR})
endif()

file(COPY ${CMAKE_SOURCE_DIR}/bweas-config.json DESTINATION ${CURRENT_BINARY_DIR})
file(MAKE_DIRECTORY ${CURRENT_BINARY_DIR}/packages)

file(GLOB SOURCE_FILES_PACKAGES ${CMAKE_SOURCE_DIR}/packages/*.json)
foreach(SOURCE_FILE IN LISTS SOURCE_FILES_PACKAGES)
    file(READ ${SOURCE_FILE} JSON_CONFIG_PACKAGE)
    string(JSON NAME_PACKAGE GET ${JSON_CONFIG_PACKAGE} "package-name")
    list(APPEND BWEAS_PACKAGES_OUTPUT "${NAME_PACKAGE}.bweas-package")
endforeach()

add_custom_target(
    BWEAS_PACKAGES ALL 
    WORKING_DIRECTORY ${CURRENT_BINARY_DIR} 
    DEPENDS ${BWEAS_PACKAGES_OUTPUT}
)

set(I_SOURCE_FILE 0)
foreach(SOURCE_FILE IN LISTS SOURCE_FILES_PACKAGES)
    list(GET BWEAS_PACKAGES_OUTPUT ${I_SOURCE_FILE} NAME_PACKAGE)
    add_custom_command(
        OUTPUT ${BWEAS_PACKAGES_OUTPUT}
        WORKING_DIRECTORY ${CURRENT_BINARY_DIR}/packages
        COMMAND ${CURRENT_BINARY_DIR}/bweas --nolog --package ${SOURCE_FILE}
        DEPENDS ${SOURCE_FILE} ${CURRENT_BINARY_DIR}/bweas
        COMMENT "Compiled ${BWEAS_PACKAGES_OUTPUT}"
    )
    set(I_SOURCE_FILE ${I_SOURCE_FILE}+1)
endforeach()

add_dependencies(BWEAS_PACKAGES ${BW_NAME_PRJ})

install(TARGETS ${BW_NAME_PRJ} DESTINATION bin)
install(FILES ${CMAKE_SOURCE_DIR}/bweas-config.json DESTINATION bin)
install(DIRECTORY ${CURRENT_BINARY_DIR}/packages DESTINATION bin)

set_target_properties(bweas PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/bin")