# bweas
> *system build*

<image src="/dev/bweas_logo.png" width=156 height=128>

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
git clone https://github.com/LuaJIT/LuaJIT
cd LuaJIT
sudo make install
```
**For lz4 and nlohmann-json**
```
cd external
git clone https://github.com/nlohmann/json
git clone https://github.com/lz4/lz4
```
> [!IMPORTANT]
> You don't need to compile it yourself(nlohmann-json and lz4), the cmake call will do it for you next..

> [!TIP]
> If you did this, then you need to set the appropriate options for cmake when building:
```
cmake -DUSER_BUILD_LUA=ON -DUSER_BUILD_JSON=ON -DUSER_BUILD_LZ4=ON ..
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
cmake -DBWEAS_BUILD_VCPKG=ON ..
cmake --build .
```
> [!TIP]
> To build tests, pass -DBWEAS_TEST=ON as an option to cmake

<br>

<img alt="Static Badge" src="https://img.shields.io/badge/build-passing-brightgree"> <img alt="Static Badge" src="https://img.shields.io/badge/release-pending-red">

## Examples
Simple configurations for assembly will be described here. These examples are for informational purposes only and do not include complex structures and new features.

```
# For example(how create variable)
set(VAR, 1)

# We create a call component - what will be called
# This also stores the name pattern of files that the current calling component may create.
create_call_component(CC_OBJ, "clang++", "object.obj")
create_call_component(CC_EXE, "clang++", "program.exe")

# Create a command template. You can use the capabilities that the current generator provides
# In this case, the nth number of commands is generated (based on the number of source files), 
# since the "feature" of the built-in generator is used: 
# FBS\_CURRENT\_INPUT\_FILE(single-generate parameter) in conjunction with FBS\_CURRENT\_OUTPUT\_FILE.)
create_templates(object_file_t, "CC_OBJ(NULL) -> OBJECTS: FBS_CURRENT_INPUT_FILE <'-o'> FBS_CURRENT_OUTPUT_FILE")
create_templates(executable_file_t, "CC_EXE(OBJECTS) -> EXECUTABLE: <{OBJECTS}> <'-o'> FBS_CURRENT_OUTPUT_FILE")

# Creating a project (there can be as many of them as you like)
# The number 1 as the second parameter indicates the programming language number
project(test, 1, "*.cpp")

# We specify the templates on the basis of which the commands will be generated
use_templates(test, "object_file_t", "executable_file_t")

# Create a target in the form of an executable file (there can be as many of them as you like)
executable(test_program, RELEASE, test)
```

