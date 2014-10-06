Planets3D
=========
is a 3D gravitational simulator inspired by Planets (planets.homedns.org)

Dependencies:
-------------
* [CMake]
* [Qt] 4.8 or 5 (Qt 5 must be 5.1 or later or desktop OpenGL build)
* [GLM]
* [TinyXml]

Building
--------
* Clone or download the source code.
* In the source folder, create a `build` folder.
* In the build folder, run `cmake .. -D<interface>=ON`, where <interface> is `PLANETS3D_QT4` or `PLANETS3D_QT5`.
* If you want to use a different generator than your platform default, add `-G <generator>` to the cmake command, with your desired generator. A list of generators can be found by running `cmake -h`.
* The project files should now be generated in `build`.

License
-------
[MIT License]


[CMake]:www.cmake.org
[Qt]:qt-project.org
[GLM]:glm.g-truc.net/
[TinyXml]:www.grinninglizard.com/tinyxml/
[MIT License]:https://github.com/chipgw/planets-3d/blob/master/LICENSE
