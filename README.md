# Simple XML parser and formatter C++ library
#### (This project is currently under active development.)

### 1. How to use the library

The simplest way to use this library is to add both simple_xml.h and simple_xml.cpp to your C++ project. 
You need a fairly modern C++ compiler which supports the C++ 20 standard to compile your C++ application 
that makes use of the included simple_xml.h and simple_xml.cpp source files.

### 1. Building the library:

First clone this github repository using the following command:
```
git clone --recurse-submodules https://github.com/atib80/simple_xml_parser
```
Enter the simple_xml_library folder, create a new build folder and change into it.

```
cd simple_xml_parser 
mkdir build
cd build
```
To build the simple_xml static library together with the tests type in the following commands in the terminal:
```
cmake -B . -S ../ -DCMAKE_BUILD_TYPE=Release
```
If you're developing on Windows launch the simple_xml_parser.sln Visual Studio 2019/2022 solution file, select    the appropriate Build mode (Release) and architecture (x64), right-click on 'Solution simple_xml_parser' and left-click on Build solution or press F7.

On Linux/Unix run the following command in the Terminal (inside the 'build' folder where CMake has generated the appropriate Makefile for your operating system):
```
make
```

### 2. Running the tests

To run the tests run the following command in the terminal:

```
./tests/simple_xml_parser_tests
```
or
```
ctest --test-dir . --output-on-failure
```


### 3. Usage of library

To use the static library simply copy or add the simple_xml.h header file, copy libsimple_xml_library.a to your C++ project's dependencies folder or add the build folder to its additional c++ libraries' path (-Ldependencies/libs) and link your C++ project with the simple_xml_library static library file (-lsimple_xml_library).