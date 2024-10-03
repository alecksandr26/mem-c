
# mem-c: A Simple Memory Allocator

## Table of Contents
1. [Overview](#overview)
2. [Installation](#installation)
3. [Getting Started](#getting-started)
   - [Basic Allocation and Freeing](#basic-allocation-and-freeing)
   - [Other Functions](#other-functions)
4. [Debugging Features](#debugging-features)
5. [License](#license)

## Overview

**mem-c** is a simple memory allocator that uses a heap data structure to manage dynamic memory allocation. The current implementation has a worst-case time complexity of `O(n * log n)` for chunk searches, but in average cases, it achieves `O(log n)` runtimes.

The allocator is designed to support various memory management features, including metadata management, chunk merging, and future plans for more sophisticated functionalities like garbage collection and arenas (user-defined memory spaces). The project is still evolving, and further optimizations are planned.

## Installation

At the moment, this package is not yet available in the AUR but may be in the future. For now, you can install **mem-c** manually via the following steps:

1. Clone the project:
   ```bash
   git clone <repository-url>
   cd mem-c
   ```

2. Build and install the package using `makepkg`:
   ```bash
   makepkg -si
   ```

This will compile the project and install it on your system.

## Getting Started

To get started using **mem-c**, include the `mem.h` header in your project and follow the examples below.

### Basic Allocation and Freeing

To allocate memory for a pointer and free it after usage:

```c
#include <mem.h>

int main() {
    int *ptr;

    // Allocate memory for an integer
    NEW(ptr);

    // Use the allocated memory
    *ptr = 42;

    // Free the allocated memory
    FREE(ptr);

    return 0;
}
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

int main() {
    int *array;

    // Allocate memory for an array of 10 integers, initialized to zero
    array = mem_calloc(sizeof(int), 10);

    // Resize the memory block
    array = mem_ralloc(array, sizeof(int) * 20);

    // Free the allocated memory
    mem_free(array);

    return 0;
}
```

## Debugging Features

In debug mode (enabled when `NDEBUG` is not defined), **mem-c** offers additional features to monitor memory usage and detect issues:

- `mem_dbg_fetch_mem_stats`: Retrieves statistics about memory usage and chunks.
- `mem_dbg_verify_ds_integrity`: Verifies the integrity of the heap data structure.
- `mem_dbg_is_freeded`: Checks if a specific address has already been freed.

Debugging example:

```c
#include <mem.h>

int main() {
    MemStats_T stats;

    // Fetch memory stats
    mem_dbg_fetch_mem_stats(&stats, 1, 1);

    // Verify data structure integrity
    mem_dbg_verify_ds_integrity();

    return 0;
}
```

## License

This project is licensed under the MIT License. See the LICENSE file for more details.
