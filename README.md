# About
This is an attempt at implementing the Wave Function Collapse algorithm for
procedural generation.


## How To Build
First, ensure you have a C and C++ compiler (clang on Windows, clang or gcc on
Linux), as well as CMake, Ninja Build and Emscripten(for web builds) installed.

Optionally, you can also install raylib locally, but cmake will
install it for you on a per project basis if you don't.

> [!NOTE]
> If you are using an IDE that uses visual studio project files such as
> VSCode, you can generate those project files by createing a folder called
> 'build' inside the project folder, open a terminal inside of the build
> folder, and run: `cmake .. -G "Visual Studio 17 2022"`

> [!NOTE]
> Web builds are not automated using the provided build script for windows
> or the visual studio project file generator and must be done manually by
> invoking emscripten from a terminal.

> [!TIP]
> you can skip inputing the build type by adding it as a command line argument
> I.E: `./build.sh debug`  
> In scriptable editors like Neovim, this can be used to automate building in
> debug mode similar to full IDEs like Visual Studio.

Then run `./build.sh` or `start build.bat` from inside the project's root
directory.
