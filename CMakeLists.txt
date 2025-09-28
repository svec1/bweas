option(USER_BUILD_LZ4 "Rep LZ4 on external" OFF)
option(USER_BUILD_JSON "Rep JSON_C on external" OFF)
option(USER_BUILD_LUA "Rep LuaJit on external" OFF)
option(BWEAS_TEST "Build of bweas test" OFF)

cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_STANDARD 23)
# set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fsanitize=address -fsanitize=undefined -g")

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
file(COPY ${CMAKE_SOURCE_DIR}/packages DESTINATION ${CURRENT_BINARY_DIR})

install(TARGETS ${BW_NAME_PRJ} DESTINATION bin)
install(FILES ${CURRENT_BINARY_DIR}/bweas-config.json DESTINATION bin)
install(DIRECTORY ${CURRENT_BINARY_DIR}/packages DESTINATION bin)

set_target_properties(bweas PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/bin")
