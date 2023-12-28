# AvUtils
AvUtils is a library of common functions and data structures for both Windows and Linux to make modern C development easier. 

## Features
### AvUtils currently offers the following features:
- ```AvString```, and various functions surrounding the length terminated string.
- various datastructures including, but not limited to, ```AvDynamicArray```, ```avQueue``` and ```AvGrid```.
- ```AvDirectory``` and ```AvFile```, for easy traversal and access to the filesystem.
- ```AvThread``` for multithreading.
- and more utility functions and datastructures

### Features currenty in progress:
- ```AvProcess```, to call other programs
- An integrated build system
- C interpreter
- Networking
- Extensive Documentation

### Features currently planned:
- Hash map implementation

## How to build
### Dependencies
- ```gcc```
- ```glibc```
- ```sh``` *(for linux)*
- ```cmd``` *(for windows)*

How to bootstrap:
```shell
gcc -o builder build/builder.c
```
Building:
```shell
./builder build avUtilities
```
The shared and static library will appear in the `./lib/` folder. A test application will appear in `./bin/`.
