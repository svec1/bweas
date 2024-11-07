# bweas
> *system build*

<image src="/dev/bweas_logo.png" width=156 height=128>

[Documentation of the internal device](https://github.com/svec1/bweas/blob/main/docs/en/main_page.md)


## Install
> ``` git clone https://github.com/svec1/bweas.git ```

> [!NOTE]
> Globally required: CMake

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
git clone https://github.com/lz4/lz4
git clone https://github.com/nlohmann/json
```
> [!IMPORTANT]
> You don't need to compile it yourself(nlohmann-json and lz4), the cmake call will do it for you next..

> [!TIP]
> If you did this, then you need to set the appropriate options for cmake when building:
```
cmake -DUSER_BUILD_LUA=ON -DUSER_BUILD_JSON=ON ..
cmake --build .
```
***

### Build on Windows
> [!NOTE]
> Required: Visual Studio 2019+
```
cd external
git clone https://github.com/LuaJIT/LuaJIT
git clone https://github.com/nlohmann/json
git clone https://github.com/lz4/lz4
VsDevCmd.bat
cd LuaJIT/src
msvcbuild.bat
exit
```
> [!TIP]
> To build LuaJit library, you need to run msvcbuild.bat in the LuaJit source folder.
> Although you can build it using mingw-make, the library will be implicit.
>
> *You don't need to build nlohmann-json and lz4*

*starting from the source folder*
```
mkdir build
cd build
cmake -DUSER_BUILD_LUA=ON -DUSER_BUILD_JSON=ON -DUSER_BUILD_LZ4=ON ..
cmake --build .
```

<br>

<img alt="Static Badge" src="https://img.shields.io/badge/build-passing-brightgree"> <img alt="Static Badge" src="https://img.shields.io/badge/release-pending-red">

## Examples
Simple configurations for assembly will be described here. These examples are for informational purposes only and do not include complex structures and new features.

```
# For example(how create variable)
set(TRUE, 1)
set(RELEASE, 1)

# We create a call component - what will be called
# This also stores the name pattern of files that the current calling component may create.
create_call_component(CC, "clang++", "program.exe")

# Create a command template. You can use the capabilities that the current generator
# provides (in this case, a file will be substituted for FBS_CURRENT_OUTPUT_FILE - because filter 1 was selected)
create_templates(template, "CC(NULL) -> EXECUTABLE: <[T_PROJECT_SRC_FILES]>, <'-o'>, FBS_CURRENT_OUTPUT_FILE:1")

# Creating a project (there can be as many of them as you like)
# The number 1 as the second parameter indicates the programming language number
project(test, 1, "main.cpp")

# We indicate that the project will use template command generation
use_it_template(test, TRUE)
use_templates(test, "template")

# create a target in the form of an executable file (there can be as many of them as you like)
executable(test_program, RELEASE, test)
```

