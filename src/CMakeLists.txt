project(bweas_slib LANGUAGES CXX C)

file(GLOB_RECURSE FILE_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
list(REMOVE_ITEM FILE_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/bweas.cpp)

if(NOT WIN32)
    find_package(BISON REQUIRED) 
    find_package(FLEX REQUIRED)

    bison_target(BW_PARSER 
             ${CMAKE_CURRENT_SOURCE_DIR}/bs/lang/parser.y 
             ${CMAKE_CURRENT_BINARY_DIR}/parser.cpp
             DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/parser.hpp)
    flex_target(BW_LEXER 
            ${CMAKE_CURRENT_SOURCE_DIR}/bs/lang/lexer.l
            ${CMAKE_CURRENT_BINARY_DIR}/lexer.cpp)

    add_flex_bison_dependency(BW_LEXER BW_PARSER)

else()
    set(FLEX_EXECUTABLE "${CMAKE_BINARY_DIR}/winflexbison-bin/win_flex.exe")
    set(BISON_EXECUTABLE "${CMAKE_BINARY_DIR}/winflexbison-bin/win_bison.exe")

    set(BISON_BW_PARSER_OUTPUTS ${CMAKE_CURRENT_BINARY_DIR}/parser.cpp ${CMAKE_CURRENT_BINARY_DIR}/parser.h)
    set(FLEX_BW_LEXER_OUTPUTS ${CMAKE_CURRENT_BINARY_DIR}/lexer.cpp)

    include(ExternalProject)
    ExternalProject_Add(
    	winflexbison
    	GIT_REPOSITORY "https://github.com/lexxmark/winflexbison.git"
    	GIT_TAG "300f48ba3e598e7a6600d724b58541fbcc3c2fa6"
    	PREFIX "${CMAKE_BINARY_DIR}/winflexbison"
	CMAKE_ARGS 
        	-DCMAKE_POLICY_VERSION_MINIMUM=3.5
        	-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/winflexbison-bin
		-Wno-dev
    	BUILD_COMMAND ${CMAKE_COMMAND} --build . --config Release
    	BUILD_BYPRODUCTS
        	"${FLEX_EXECUTABLE}"
        	"${BISON_EXECUTABLE}"
    )

    add_custom_target(BWLANG_PARSER_LEXER_SOURCE
  	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  	DEPENDS ${BISON_BW_PARSER_OUTPUTS} ${FLEX_BW_LEXER_OUTPUTS}
  	VERBATIM
    )

    add_dependencies(BWLANG_PARSER_LEXER_SOURCE winflexbison)

    add_custom_command(
    	OUTPUT ${BISON_BW_PARSER_OUTPUTS}
    	COMMAND ${BISON_EXECUTABLE} -d ${CMAKE_CURRENT_SOURCE_DIR}/bs/lang/parser.y -o ${CMAKE_CURRENT_BINARY_DIR}/parser.cpp
    	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/bs/lang/parser.y
    	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    	COMMENT "Bison launched"
    )

    add_custom_command(
    	OUTPUT ${FLEX_BW_LEXER_OUTPUTS}
    	COMMAND ${FLEX_EXECUTABLE} -o ${CMAKE_CURRENT_BINARY_DIR}/lexer.cpp ${CMAKE_CURRENT_SOURCE_DIR}/bs/lang/lexer.l
    	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/bs/lang/lexer.l
    	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    	COMMENT "Flex launched"
    )

endif()

add_library(${BWEAS_SLIB} STATIC ${FILE_SOURCE} ${BISON_BW_PARSER_OUTPUTS} ${FLEX_BW_LEXER_OUTPUTS})

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
        add_subdirectory(../external/json)
        message(STATUS "Selected nlohmann_json in folder - external")
    else()
        find_package(nlohmann_json REQUIRED)
    endif()

    # LZ4
    if(USER_BUILD_LZ4)
        set(LZ4_BUILD_CLI OFF)
        add_subdirectory(../external/lz4/build/cmake)
        message(STATUS "Selected LZ4 in folder - external")
    else()
        find_library(LZ4_LIBRARY NAMES lz4)
    endif()

    set(BWEAS_LIBS ${LuaJIT_LIBRARIES} nlohmann_json::nlohmann_json lz4)
    set(BWEAS_INCLUDE ${NLOHMANN_JSON_INCLUDE_INSTALL_DIR} ${LuaJIT_SRC_DIR} CACHE STRING "All header-files for bweas")
endif()

set(BWEAS_INCLUDE ${BWEAS_INCLUDE} "bs/")

target_link_libraries(${BWEAS_SLIB} PRIVATE ${BWEAS_LIBS})
target_precompile_headers(${BWEAS_SLIB} PUBLIC bs/bwaliases.hpp bs/utils/lua_wrapper.hpp bs/bwstructs_context.hpp bs/lang/var.hpp bs/lang/scope.hpp)

include_directories(${BWEAS_INCLUDE} ${CMAKE_CURRENT_BINARY_DIR})
