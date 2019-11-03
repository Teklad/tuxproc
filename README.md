# tuxproc
An external process memory library for Linux

### Description

This library is designed to make use of `process_vm_readv` and `process_vm_writev` easier when working with external process memory
on Linux.

Please refer to the commented code for reference.

### Building

Build commands in repository root:

```
mkdir build
cd build
cmake ..
make && sudo make install
```

To run a program linked with the library, you must add `/usr/local/lib` to `LD_LIBRARY_PATH`

### Demo Program

Navigate to the `demo` directory in the repository root and read the `README` for more information on how to run a small demo program, in which the value of a variable in a remote process is modified.

### Usage example

```cpp
#include <cstdio>
#include <unistd.h>
#include <tuxproc/process.h>

uintptr_t address = "0x123456";

int main()
{
    TuxProc::Process process;
    while (!process.attach("process_name") || !process.isRunning()) {
        usleep(10000);
    }

    process.parseMaps();

    // first region of mapped file, that has read access
    TuxProc::Region* region = process.getRegion("target_file.so", 0, TuxProc::READ);

    if (region) {
        auto someInt = process.read<int>(address);
        printf("%d\n", someInt);
    }
}
```
