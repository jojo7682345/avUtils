# AvUtils
AvUtils is a library of common functions and data structures for C for both Windows and Linux.

## Contents
### Typedefs
Some additional typedefs are provided.
- `uint` - *32-bit unsigned integer*
- `uint32` - *32-bit unsigned integer*
- `uint64` - *64-bit unsigned integer*
- `uint16` - *16-bit unsigned integer*
- `uint8` - *8-bit unsigned integer*
- `byte` - *8-bit unsigned integer*

- `int32` - *32-bit signed integer*
- `int64` - *64-bit signed integer*
- `int16` - *16-bit signed integer*
- `int8` - *8-bit signed integer*

- `bool8` - *8-bit unsigned integer*
- `bool32` - *32-bit unsigned integer*

If these typedefs cause name conflicts. Use:
```C
#define AV_DISABLE_TYPES
```
before including. Then use:
```C
#undef uint32
```
for any conflicting types

### Memory
Utilities for memory allocation
- `AvAllocate` - *Allocate memory on the heap*
- `AvCallocate` - *Allocate memory on the heap*
- `AvReallocate` - *Reallocate memory on the heap*
- `AvFree` - *Free memory from the heap*
- (comming soon) AvPrintMemoryLeaks - *Used for finding memory leaks*

### Datastructures
These are structures used to store data in different ways.
- `AvGrid` - *A structure where the data elements can be accessed by x and y coordinates*
- `AvQueue` - *A FIFO structure where elements can be pushed to and popped from*
- `AvTable` - *A structure with multiple columns which can hold data, can be access through row and colunm number*
- `AvDynamicArray` - *A structure which can be used to dynamicaly store data. Automatically grows to accomedate new elements*
- (in progress) `AvMap`

### Threading
These are used to control threads
- `AvThread` - *Used for calling functions in parralel*
- `AvMutex` - *Used for thread synchronisation*
- (in progress) `AvThreadPool`

### Networking
Used to easily communicate over the network
- (in progress) `AvServer`
- (in progress) `AvSocket`
- (in progress) `avConnect`

### Logging
Debug and logging utilities
- `avAssert` - *Assert a statement, used for debugging purposes*


## Naming
Everything start with the AV prefix. Functions start with a lowercase `av`. Handles and types start with `Av` and constants, enums and definitions start with `AV`.

Functions have the following format: `av` `Type` `Operation` `(`	*`arguments`* `,` *`handle`* `)`

Handles have the follwing format: `Av` `Type`.

Constants have the follwing format: `AV` `_` `DEFINITION_NAME`

some function have wrapper defined, this is for debugging purposes

## Usage
Most datastructures and handlers can be used using the following paradigm
```c
AvGrid grid;
avGridCreate(sizeof(Type), width, height, &grid);

avGridRead(x, y, grid);

avGridDestroy(grid);
```



## How to build
How to bootstrap:
```shell
gcc -o builder build/builder.c
```
Building:
```shell
./builder build avUtilities
```
The shared and static library will appear in the `./lib/` folder. A test application will appear in `./bin/`.
