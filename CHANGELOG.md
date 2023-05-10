# VIPR Emulator Changelog

## Version 0.2

- Changed the message when GL Context creation fails in all OpenGL-based renderers to use the one supplied by SDL2.

- Fixed the OpenGL 3.0 renderer as it had numerous bugs.

- Added the ability to load a custom ROM file.  As a result, you are no longer forced to supply the 'VIP.rom' file for either the operating system or a custom ROM.

- Added a new status to the Memory Transfer menu when you're about to overwrite an existing file.  All you have to do is activate the "Transfer" option the second time to overwrite the file.

- Most keys should now work with num lock turned on during input.

- Added the ability to change output audio device to allow switching between different output audio devices and reduce the need to restart the whole application.

- Added the ability to change the main volume of the application.  Can be accessed through the "Emulator Options" menu.

- VP-595 Simple Sound Board has been implemented as one of the first supported expansion boards.  When this board is enabled, it turns off the base tone generator.

- VP-585 Expansion Keypad Interface has been implemented to enable support for two keypads without the need for the color board.

- Second hexadecimal keypad is now supported whenever the VP-585 Expansion Keypad Interface or VP-590 Color Board is turned on.

- Audio is now properly paused as well as releasing any key that was held when you enter back to the main menu.

- VP-590 Color Board has been implemented to provide color and enable support for two keypads.  CDP1861 code had to be partially redesigned to accommodate this particular expansion and preserve the original functionality.  On top of that, all the renderers had to be upgraded to add color support.

## Version 0.1

- Initial Release
