# BookFiler SSH Module
SSH library based on `libssh2`. The main feature is the SSH tunnel, which will allow mysql to be connected to through an SSH connection. This is useful for shared hosting with cPanel, which sometimes will disable direct mysql access.

This module is meant to be used with the module system for BookFiler software suite of programs. The module system is designed to separately compile modules, so that the end application doesnt require a large compilation and linking step. The modules are similar to dynamic linked libraries, except that they don't need to be linked to the main application. This way if a module is missing, the main application can give a more detailed and graphically friendly message indicating missing modules rather than an ugly operating system error window.

## Dependencies

| Library				| Purpose					|
| :---					| :---						|
| Boost::Asio			| Networking 				|
| rapidJSON				| JSON parser				|
| linssh2				| SSH2						|

# Table of Contents

<!--ts-->
   * [Build Instructions](#build-instructions)
   * [Usage Instructions](#usage-instructions)
      * [Creating an SSH tunnel](#Creating-an-SSH-tunnel)
         * [C++](#c)
      * [Creating an SSH connection](#Creating-an-SSH-connection)
         * [C++](#c-1)
      * [Cookies](/docs/cookies.md)
   * [Dependency](#dependency)
   * [License](#license)
<!--te-->

# Build Instructions

## Windows - MinGW-w64
Install MSYS2<BR>
Then, install GCC, cmake, git and dependencies. Confirmed working with Windows 10 as of 11/19/2020.
```shell
pacman -Syu
pacman -S mingw-w64-x86_64-gcc git make mingw-w64-x86_64-cmake
pacman -Rns cmake
# restart MSYS2 so that we use the mingw cmake
pacman -S mingw-w64-x86_64-boost mingw-w64-x86_64-openssl mingw-w64-x86_64-libssh2 mingw-w64-x86_64-re2 mingw-w64-x86_64-zlib mingw-w64-x86_64-fmt
```
Build:
```shell
git clone https://github.com/bradosia/BookFiler-Module-HTTP
cd BookFiler-Module-HTTP
mkdir build
cd build
cmake -G "MSYS Makefiles" ../
make
```

## Linux Ubuntu
Install GCC, cmake, git and dependencies. Confirmed working with Ubuntu 20.04 as of 11/8/2020.
```shell
sudo apt-get update
sudo apt install build-essential gcc-multilib g++-multilib cmake git
sudo apt install libboost-all-dev libssl-dev libre2-dev zlib1g libfmt-dev
```
Build:
```shell
git clone https://github.com/bradosia/BookFiler-Module-HTTP
cd BookFiler-Module-HTTP
mkdir build
cd build
cmake ../
make
```
 
# Usage Instructions

## Creating an SSH tunnel

### c++
```cpp
```

## Creating an SSH connection

### c++
```cpp
```

## Dependency

| Library				| License						| Link														|
| :---					| :---							| :---														|
| Boost::Asio			| Boost Software License 1.0 	| https://github.com/boostorg/asio 							|
| rapidJSON				| MIT							| https://github.com/Tencent/rapidjson						|
| libssh2				| attribute						| https://github.com/openssl/openssl						|

# License
The library is licensed under the MIT License: <BR>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
