Planets3D
=========
is a 3D gravitational simulator inspired by Planets (planets.homedns.org)

Dependencies:
-------------
* [CMake]
* [GLM]
* [TinyXml]

For Qt interface:
* [Qt] 4.8 or 5 (Qt 5 must be 5.1 or later or desktop OpenGL build)

For SDL interface:
* [SDL] (2.0)
* [SDL_image]

Building
--------
* Clone or download the source code.
* In the source folder, create a `build` folder.
* In the build folder, run `cmake .. -D<interface>=ON`, where `<interface>` is `PLANETS3D_QT4`, `PLANETS3D_QT5`, or `PLANETS3D_SDL`.
* If you want to use a different generator than your platform default, add `-G <generator>` to the cmake command, with your desired generator. A list of generators can be found by running `cmake -h`.
* The project files should now be generated in `build`.

License
-------
[MIT License]


[CMake]:http://www.cmake.org
[Qt]:http://qt-project.org
[GLM]:http://glm.g-truc.net/
[TinyXml]:http://www.grinninglizard.com/tinyxml/
[SDL]:http://www.libsdl.org
[SDL_image]:http://www.libsdl.org/projects/SDL_image
[MIT License]:LICENSE
