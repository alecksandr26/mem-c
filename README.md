
# Mem C (mem-c)

## Table of Contents
1. [Overview](#overview)
2. [Installation](#installation)
3. [Getting Started](#getting-started)
   - [Basic Allocation and Freeing](#basic-allocation-and-freeing)
   - [Other Functions](#other-functions)
4. [Debugging Features](#debugging-features)
5. [License](#license)

## Overview

**mem-c** is a simple memory allocator that uses a heap data structure with the **mmap** Linux syscall to manage dynamic memory allocation, *just for educational purpose*. The current implementation has a worst-case time complexity of `O(n * log n)` for chunk searches, but in average cases, it achieves `O(log n)` runtimes.

The allocator is designed to support various memory management features, memory pagination, chunk merging, and future plans for more sophisticated functionalities like garbage collection and arenas (user-defined memory spaces). The project is still evolving, and further optimizations are planned.

Currently, the allocator likely only works on **64-bit** CPU machines.

## Installation

At the moment, this package is not yet available in the AUR but may be in the future. For now, you can install **mem-c** manually via the following steps:

1. Clone the project or download a relesed version of the project:
   ```bash
   git clone <repository-url>
   cd mem-c
   ```

2. Build and install the package using `makepkg`:
   ```bash
   makepkg -si
   ```

This will compile the project and install it on your Arch Linux system.
If you are using a different distribution, you can run:

1. Run `make` with the compilation
   ```
   make compile
   ```
2. Then, install the header file and the library:
   ```
   cp include/mem.h /path/you/want/to/install
   cp build/lib/libmem.so /path/you/want/to/install
   ```
   
## Getting Started

To get started using **mem-c**, include the `mem.h` header in your project and follow the examples below.

### Basic Allocation and Freeing

To allocate memory for a pointer and free it after usage:

```c
#include <mem.h>

struct Person {
    char name[100];
    int age;
};

int main(void)
{
    strcut Person *ptr;

    // Allocate memory for a new struct Person, with the macro `NEW`
    NEW(ptr);

    // Use the allocated memory
    ptr->age = 10;

    // More code here ...

    // Free the allocated memory, with the macro `FREE`
    FREE(ptr);

    return 0;
}
```

### Compiling

To compile your code, you must link the library using the `-lmem` flag:
```c
cc yourcode.c -lmem
```

### Other Functions

**mem-c** provides several other functions for memory management:

- `mem_alloc`: Allocates a specified number of bytes.
- `mem_ralloc`: Reallocates memory for a given address.
- `mem_calloc`: Allocates memory for an array of objects, initializing them to zero.
- `mem_free`: Frees memory associated with a given address.

Example usage:

```c
#include <mem.h>

int main(void)
{
    int *array;

    // Allocate memory for an array of 10 integers, initialized to zero
    array = mem_calloc(sizeof(int), 10);

    // More code here ...

    // Resize the memory block
    array = mem_ralloc(array, sizeof(int) * 20);

    // More code here ...

    // Free the allocated memory
    mem_free(array);

    return 0;
}
```

## Debugging Features

In debug mode (enabled when `NDEBUG` is not defined), **mem-c** offers additional features to monitor memory usage and detect issues:

- `mem_dbg_fetch_mem_stats:` Retrieves statistics about memory usage and chunks. It also logs this information to the given file descriptor based on the specified verbosity level.
  - Level 0 will not print anything.
  - Level 1 will print only the main statistics.
  - Level 2 will print all pages and their information.
  - Level 3 will print each allocated chunk.

  The *verbosity* is given as the *second argument*, and the *third* argument will be the *file descriptor*.

- `mem_dbg_verify_ds_integrity`: Verifies the integrity of the heap data structure.
- `mem_dbg_is_freeded`: Checks if a specific address has already been freed, useful for asserting addresses.

Debugging example:

```c
#include <mem.h>


void foo(void *ptr)
{
    assert(!mem_dbg_is_freeded(ptr), "Should be able to be used");

    // Do something with that pointer ...
}


int main(void)
{
    MemStats_T stats;

    // Several allocations here ...

    // Fetch memory stats. This will log information at verbosity level 1 and output it to file descriptor 1 (stdout).
    mem_dbg_fetch_mem_stats(&stats, 1, 1);

    // Verify data structures integrity
    mem_dbg_verify_ds_integrity();

    // Free all the allocs ...

    return 0;
}
```

## License

This project is licensed under the MIT License. See the LICENSE file for more details.
