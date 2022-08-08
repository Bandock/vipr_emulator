# VIPR Emulator
A cross-platform COSMAC VIP emulator capable of running on a wide variety of systems.

## Usage
In order to use this emulator, you need to supply the operating system ROM with the file name `VIP.rom`.  The naming scheme is likely to change in the future.  This ROM shouldn't be too hard to find or assemble.

Only means of transferring data to and from files right now is through the `Memory Transfer` feature.  Be aware that when storing, there is currently no prompt right now for overwriting (which will change in the near future).

You can specify 1-32KB of RAM.

## Currently Supported Renderers
- OpenGL 2.1 (Should run on most hardware, though it does take advantage of some extensions.)
- OpenGL ES 2.0 (This allows it to run on various systems such as nearly any Raspberry Pi.)
- OpenGL ES 3.0 (This allows it to run on more modern embedded devices and even some systems like the Raspberry Pi 4.  First renderer that was built as the development machine was a Raspberry Pi 4.)

## Requirements for Building
- [CMake](https://www.cmake.org/download/) (at least 3.10)
- [fmt](https://github.com/fmt)
- [libmsbtfont](https://github.com/Bandock/libmsbtfont) (Requires at least 0.2.0, latest release recommended)
- [SDL2](https://www.libsdl.org/download-2.0.php) (Latest stable development versions should work fine)
- [GLEW](http://glew.sourceforget.net) (If you're compiling with OpenGL 2.1 and soon OpenGL 3.0 renderer support)
- C++ Compiler with C++20 Support
