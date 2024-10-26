# bweas
> *system build*

<image src="/dev/bweas_logo.png" width=156 height=128>

[Documentation of the internal device](https://github.com/svec1/bweas/blob/main/docs/en/main_page.md)


## Install
> ``` git clone https://github.com/svec1/bweas.git ```

> [!NOTE]
> Globally required: CMake

### Build on Linux
Before building, you should install the LuaJit library.
##### *debian based(for example)*
```
sudo apt install luajit
```
You can also copy the LuaJit repository and follow these steps:
> [!NOTE]
> Required: Make
```
cd external
git clone https://github.com/LuaJIT/LuaJIT
cd luajit
make
```

Default build with cmake
```
mkdir build
cd build
cmake ..
cmake --build .
```

### Build on Windows
> [!NOTE]
> Required: Visual Studio 2019+
```
cd external
git clone https://github.com/LuaJIT/LuaJIT
cmd /E:ON /K ""C:\Program Files (x86)\Intel\oneAPI\setvars.bat" --force"
cd luajit/src
msvcbuild.bat
exit
```
> [!TIP]
> To build LuaJit library, you need to run msvcbuild.bat in the LuaJit source folder.
> Although you can build it using mingw-make, the library will be implicit

*starting from the source folder*
```
mkdir build
cd build
cmake ..
cmake --build .
```

<br>

<img alt="Static Badge" src="https://img.shields.io/badge/build-passing-brightgree"> <img alt="Static Badge" src="https://img.shields.io/badge/release-pending-red">
