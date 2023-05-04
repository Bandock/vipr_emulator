# VIPR Emulator
A cross-platform COSMAC VIP emulator capable of running on a wide variety of systems.

## Usage
When you start this emulator, you'll need to load up a ROM file.  This can include the operating system ROM as well as custom ROMs out there.  You can load ROM files up to 32KB.  Shouldn't be hard to find the operating system ROM or even ways to build it yourself if you want to use the monitor.

Only means of transferring data to and from files right now is through the `Memory Transfer` feature.  If you're storing into a file that already exists, the "Overwrite?" transfer status will be displayed.  This is to prevent accidental overwrites.  All you have to do is activate the "Transfer" option again to overwrite the file you're targeting.

You can specify 1-32KB of RAM.

You can also turn on different expansion boards that allow for increased capabilities such as color, additional sound, keypads, and more.  This can be found in the `Machine Options` menu, which also means the machine must be turned off before you can toggle any expansion board.

To change between the `RUN` and `RESET` state, just press `RETURN`.  If you want to access the operating system, hold the `C` (mapped to `4` right now) key and press `RETURN`.  That should allow you to use the operating system like the original COSMAC VIP.

## Key Bindings
Original COSMAC VIP Hex Keyboard Layout:
|0|1|2|3|
|-|-|-|-|
|`1`|`2`|`3`|`C`|
|`4`|`5`|`6`|`D`|
|`7`|`8`|`9`|`E`|
|`A`|`0`|`B`|`F`|

Current Key Bindings:
|0|1|2|3|
|-|-|-|-|
|`1`|`2`|`3`|`4`|
|`Q`|`W`|`E`|`R`|
|`A`|`S`|`D`|`F`|
|`Z`|`X`|`C`|`V`|

## Expansion Board Support
- VP-595 Simple Sound Board (Replaces the base tone generator when turned on.)

## Currently Supported Renderers
- OpenGL 2.1 (Should run on most hardware, though it does take advantage of some extensions.)
- OpenGL 3.0 (Should run on DX10 class hardware at least.  It too also takes advantage of some extensions.)
- OpenGL ES 2.0 (This allows it to run on various systems such as nearly any Raspberry Pi.)
- OpenGL ES 3.0 (This allows it to run on more modern embedded devices and even some systems like the Raspberry Pi 4.  First renderer that was built as the development machine was a Raspberry Pi 4.)

## Requirements for Building
- [CMake](https://www.cmake.org/download/) (at least 3.10)
- [fmt](https://github.com/fmtlib/fmt)
- [libmsbtfont](https://github.com/Bandock/libmsbtfont) (Requires at least 0.2.0, latest release recommended)
- [SDL2](https://www.libsdl.org/download-2.0.php) (Latest stable development versions should work fine)
- [GLEW](http://glew.sourceforge.net) (If you're compiling with OpenGL 2.1 and OpenGL 3.0 renderer support)
- C++ Compiler with C++20 Support
