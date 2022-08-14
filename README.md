# VIPR Emulator
A cross-platform COSMAC VIP emulator capable of running on a wide variety of systems.

## Usage
In order to use this emulator, you need to supply the operating system ROM with the file name `VIP.rom`.  The naming scheme is likely to change in the future.  This ROM shouldn't be too hard to find or assemble.

Only means of transferring data to and from files right now is through the `Memory Transfer` feature.  Be aware that when storing, there is currently no prompt right now for overwriting (which will change in the near future).

You can specify 1-32KB of RAM.

To change between the `RUN` and `RESET` state, just press `RETURN`.  If you want to access the operating system, hold the `C` (mapped to `4` right now) key and press `RETURN`.  That should allow you to use the operating system like the original COSMAC VIP.

## Key Bindings
Original COSMAC VIP Hex Keyboard Layout:
|`1`|`2`|`3`|`C`|
|`4`|`5`|`6`|`D`|
|`7`|`8`|`9`|`E`|
|`A`|`0`|`B`|`F`|

## Currently Supported Renderers
- OpenGL 2.1 (Should run on most hardware, though it does take advantage of some extensions.)
- OpenGL 3.0 (Should run on DX10 class hardware at least.  It too also takes advantage of some extensions.)
- OpenGL ES 2.0 (This allows it to run on various systems such as nearly any Raspberry Pi.)
- OpenGL ES 3.0 (This allows it to run on more modern embedded devices and even some systems like the Raspberry Pi 4.  First renderer that was built as the development machine was a Raspberry Pi 4.)

## Requirements for Building
- [CMake](https://www.cmake.org/download/) (at least 3.10)
- [fmt](https://github.com/fmt)
- [libmsbtfont](https://github.com/Bandock/libmsbtfont) (Requires at least 0.2.0, latest release recommended)
- [SDL2](https://www.libsdl.org/download-2.0.php) (Latest stable development versions should work fine)
- [GLEW](http://glew.sourceforget.net) (If you're compiling with OpenGL 2.1 and OpenGL 3.0 renderer support)
- C++ Compiler with C++20 Support
