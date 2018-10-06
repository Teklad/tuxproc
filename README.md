# tuxproc
An external process memory library for Linux

### Description
This library is designed to make use of process_vm_readv and process_vm_writev easier when working with external process memory
on Linux.

Documentation can be found [here](https://teklad.github.io/tuxproc/class_tux_proc_1_1_process.html)

### Usage example

The simple usage example to get started with using this library can be found below.

```cpp
#include <tuxproc/process.h>
#include <iostream>
#include <unistd.h>

int main()
{
    TuxProc::Process process;
    while (!process.Attach("some_process_name")) {
        usleep(10000);
    }
    
    while (!process.GetRegion("some_required_region.so")) {
        process.ParseMaps();
        usleep(10000);
    }
    
    auto someInt = process.Read<int>(someAddress);
    std::cout << someInt;
}
```
