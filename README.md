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
cd luajit
make
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
cmd /E:ON /K ""C:\Program Files (x86)\Intel\oneAPI\setvars.bat" --force"
cd luajit/src
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
