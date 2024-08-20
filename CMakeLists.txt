cmake_minimum_required(VERSION 3.20)

set(BW_NAME_PRJ bweas)

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

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Ox")

add_executable(${BW_NAME_PRJ} ${FILE_SRC})

if(WIN32)
    message("Enabling the detours lib(Microsoft)")
    if (${CMAKE_SIZEOF_VOID_P} STREQUAL 4)
        message("Selected lib 32 bit build")
        target_link_libraries(${BW_NAME_PRJ} ${CMAKE_SOURCE_DIR}/external/detours/lib.X86/detours.lib)
    elseif (${CMAKE_SIZEOF_VOID_P} STREQUAL 8)
        message("Selected lib 64 bit build")
        target_link_libraries(${BW_NAME_PRJ} ${CMAKE_SOURCE_DIR}/external/detours/lib.X64/detours.lib)
    endif ()
    include_directories("external/detours/include")
else()
    message("For this platform, there is no protection for plug-ins and modules")
endif()