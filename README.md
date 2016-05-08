Planets3D
=========
is a 3D gravitational simulator.

Dependencies:
-------------
* [CMake]
* [GLM]
* [TinyXml]

For Qt interface:
* [Qt] 5.4 or later.
* [SDL] 2.0 or greater. (Optional for controller support. Add `-DPLANETS3D_QT_USE_SDL_GAMEPAD=On` to enable)

For SDL interface:
* [SDL] 2.0 or greater.
* [SDL_image] 2.0 or greater.

Building
--------
* Clone or download the source code.
* In the source folder, create a `build` folder.
* In the build folder, run `cmake .. -D<interface>=ON`, where `<interface>` is `PLANETS3D_QT5` or `PLANETS3D_SDL`.
* If you want to use a different generator than your platform default, add `-G <generator>` to the cmake command, with your desired generator. A list of generators can be found by running `cmake -h`.
* The project files should now be generated in `build`.

Web interface using Emscripten:
===============================
[Demo]

Dependencies:
-------------

* [CMake]
* [GLM]
* [Emscripten]

Building
--------
* Clone or download the source code.
* In the source folder, create a `build` folder.
* In the build folder, run `cmake .. -DCMAKE_TOOLCHAIN_FILE=<EmscriptenRoot>/cmake/Modules/Platform/Emscripten.cmake -G "<generator>"`, where `<EmscriptenRoot>` is the path to the Emscripten installation, and `<generator>` is `Unix Makefiles` on Linux & OSX and `MinGW Makefiles` on Widnows.
* Then run `make` or `mingw32-make`.

License
-------
[MIT License]


[CMake]:http://www.cmake.org
[Qt]:http://www.qt.io
[GLM]:http://glm.g-truc.net/
[TinyXml]:http://www.grinninglizard.com/tinyxml/
[SDL]:http://www.libsdl.org
[SDL_image]:http://www.libsdl.org/projects/SDL_image
[Emscripten]:http://kripken.github.io/emscripten-site/
[Demo]:http://chipgw.github.io/planets/
[MIT License]:LICENSE
