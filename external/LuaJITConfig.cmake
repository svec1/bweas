set(LUAJIT_SRC_DIR "${CMAKE_CURRENT_LIST_DIR}/LuaJIT/src" CACHE STRING "LuaJIT include directory")

if(NOT EXISTS "${LUAJIT_SRC_DIR}")
    message(FATAL_ERROR "LuaJIT directory not found: ${LUAJIT_SRC_DIR}")
endif()

if(WIN32)
    set(LUAJIT_NAME_LIB "lua51")
    set(LUAJIT_LIBRARIES "${CMAKE_CURRENT_LIST_DIR}/LuaJIT/src/${LUAJIT_NAME_LIB}.lib" CACHE STRING "LuaJIT library")
else()
    set(LUAJIT_NAME_LIB "libluajit")
    set(LUAJIT_LIBRARIES "${CMAKE_CURRENT_LIST_DIR}/LuaJIT/src/${LUAJIT_NAME_LIB}.a" CACHE STRING "LuaJIT library")
endif()

if(NOT EXISTS "${LUAJIT_LIBRARIES}")
    message(FATAL_ERROR "LuaJIT library not found: ${LUAJIT_LIBRARIES}")
endif()

set(LuaJIT_SRC_DIR ${LUAJIT_SRC_DIR} CACHE STRING "LuaJIT include directories")
set(LuaJIT_LIBRARIES ${LUAJIT_LIBRARIES} CACHE STRING "LuaJIT libraries")
set(LuaJIT_NAME_LIBRARIES ${LUAJIT_NAME_LIB} CACHE STRING "LuaJIT name libraries")


mark_as_advanced(LuaJIT_SRC_DIR LuaJIT_LIBRARIES LuaJIT_NAME_LIBRARIES)