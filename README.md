Planets3D
=========
is a 3D gravitational simulator.

Dependencies:
-------------
* [CMake] 3.2 or greater.
* [GLM]
* [TinyXml]

For Qt interface:
* [Qt] 5.4 or greater.
* [SDL] 2.0 or greater. (Optional for controller support. Add `-DPLANETS3D_QT_USE_SDL_GAMEPAD=ON` to enable)

For SDL interface:
* [SDL] 2.0 or greater.
* [SDL_image] 2.0 or greater.
* [ImGui] 1.49 or greater. Place source code in `<project root>/imgui`.
* [NativeFileDialog] Optional for open/save dialogs, use `-DPLANETS3D_WITH_NFD=ON` to enable.

Building
--------
* Clone or download the source code.
* For SDL interface extract ImGui source files in an `imgui` folder.
* In the source folder, create a `build` folder.
* In the build folder, run `cmake .. -D<interface>=ON`, where `<interface>` is `PLANETS3D_QT5` or `PLANETS3D_SDL`.
* If you want to use a different generator than your platform default, add `-G <generator>` to the cmake command, with your desired generator. A list of generators can be found by running `cmake -h`.
* (Optional) To build TinyXML from source (Useful if you get TinyXML related link errors on Windows) place the source files in `<project root>/tinyxml` and add `-DPLANETS3D_BUILD_TINYXML=ON` to the cmake command.
* The project files should now be generated in `build`.

Web interface using Emscripten:
===============================
[Demo]

Dependencies:
-------------

* [CMake] 3.2 or later.
* [GLM]
* [Emscripten]
* [SDL] 2.0 or greater. (Will be auto-downloaded by Emscripten)

Building
--------
* Clone or download the source code.
* In the source folder, create a `build` folder.
* In the build folder, run `cmake .. -DCMAKE_TOOLCHAIN_FILE=<EmscriptenRoot>/cmake/Modules/Platform/Emscripten.cmake -G "<generator>"`, where `<EmscriptenRoot>` is the path to the Emscripten installation, and `<generator>` is `Unix Makefiles` on Linux & OSX and `MinGW Makefiles` on Windows.
* Then run `make` or `mingw32-make`.

License
-------
[MIT License]


[CMake]:https://www.cmake.org
[Qt]:https://www.qt.io
[GLM]:http://glm.g-truc.net/
[TinyXml]:http://www.grinninglizard.com/tinyxml/
[SDL]:http://www.libsdl.org
[SDL_image]:http://www.libsdl.org/projects/SDL_image
[ImGui]:https://github.com/ocornut/imgui
[NativeFileDialog]:https://github.com/mlabbe/nativefiledialog
[Emscripten]:http://kripken.github.io/emscripten-site/
[Demo]:http://chipgw.github.io/planets/
[MIT License]:LICENSE
