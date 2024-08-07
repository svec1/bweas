cmake_minimum_required(VERSION 3.20)

set(BW_NAME_PRJ bweas)

set(CMAKE_CXX_STANDARD 17)
project(${BW_NAME_PRJ} LANGUAGES CXX C)

file(GLOB MAIN "src/main.cpp" "src/mdef.cpp")

file(GLOB KERNEL_LL "src/kernel/low_level/*.c")
file(GLOB KERNEL_HL "src/kernel/high_level/*.cpp" "src/kernel/*.hpp")

file(GLOB KERNEL ${KERNEL_LL} ${KERNEL_HL})
file(GLOB HAX "src/bs/console_arg/hax.cpp")
file(GLOB LANG "src/bs/lang/*.hpp" "src/bs/lang/*.cpp")
file(GLOB MODULE "src/bs/smodule/*.hpp" "src/bs/smodule/*.cpp" "src/bs/smodule/srl/*.hpp" "src/bs/smodule/srl/*.cpp")

file(GLOB BS "src/bs/*.cpp" ${LANG})

file(GLOB FILE_SRC ${MAIN} ${KERNEL} ${BS} ${HAX} ${MODULE})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Ox")

add_executable(${BW_NAME_PRJ} ${FILE_SRC})