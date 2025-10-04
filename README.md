# bweas
> *system build*

[Documentation of the internal device](https://github.com/svec1/bweas/blob/main/docs/en/main_page.md)


## Install
> ``` git clone https://github.com/svec1/bweas.git ```

> [!NOTE]
> Globally required: CMake, C++ compiler(Clang is recommended)

### Build on Linux
Before building, you should install the LuaJit and nlohmann-json libraries.
##### *debian based(for example)*
```
sudo apt install luajit nlohmann-json3-dev liblz4-dev
```
#### Default build with cmake:
```
mkdir build
cd build
cmake ..
cmake --build .
```
***
#### Build with clone rep of libraries
You can also clone the LuaJit or nlohmann/json repositories and follow these steps:
> [!NOTE]
> For LuaJit:
> Required: Make
```
cd external
git clone https://github.com/nlohmann/json
git clone https://github.com/LuaJIT/LuaJIT
cd LuaJIT
sudo make install
```
> [!TIP]
> If you did this, then you need to set the appropriate options for cmake when building:
```
cmake -DUSER_BUILD_LUA=ON -DUSER_BUILD_JSON=ON ..
cmake --build .
```
***

### Build on Windows
*starting from the source folder*
> [!NOTE]
> Required: CMake, VCPKG
```
git clone https://github.com/microsoft/vcpkg.git
vcpkg\bootstrap-vcpkg.bat
vcpkg\vcpkg integrate install
mkdir build
cd build
cmake -DCMAKE_TOOLCHAIN_FILE=..\vcpkg\scripts\buildsystems\vcpkg.cmake ..
cmake --build . --config Release
```
> [!TIP]
> To build tests, pass -DBWEAS_TEST=ON as an option to cmake

<br>

<img alt="Static Badge" src="https://img.shields.io/badge/build-passing-brightgree"> <img alt="Static Badge" src="https://img.shields.io/badge/release-pending-red">

## Examples
Simple configurations for assembly will be described here. These examples are for informational purposes only and do not include complex structures and new features.

```
import 'base'

# Create a command template. You can use the capabilities that the current generator provides
# In this case, the nth number of commands is generated (based on the number of source files), 
# since the "feature" of the built-in generator is used: 
# INPUT_FILE(single-generate parameter) in conjunction with OUTPUT_FILE.)
t_obj: ctemplate = {str =' clang++:{}.o() -> OBJECTS: INPUT_FILE -o OUTPUT_FILE [compile_flags]'}
t_exe: ctemplate = {str =' clang++:{target_name}(OBJECTS) -> target: {OBJECT} -o OUTPUT_FILE'}

# Creating a target object that will continue to be built
my_target: target = {
                    type = executable,
                    extenstion = cpp,
                    templates = ['t_exe', 't_obj'],
                    compile_flags = ['-std=gnu++23', '-Wall'],
                    source_files = file(get_files, 'main.cpp')
}

# Registering the target object for the build
build(my_target)

```

