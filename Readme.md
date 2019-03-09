# etherdream-glsl

Draw an animation using [Ether Dream (laser DAC)](https://www.ether-dream.com/) which follows points generated by a shader.

## Build

Generate your build files using [Premake 5](https://premake.github.io/).

### Windows

The recommanded way is to install [Visual Studio 2017](https://visualstudio.microsoft.com/) (there's a free version).

Open a console in the project directory and run:

    premake5 vs2017

Open the generated _build/etherdream-glsl.sln_ and compile.

## Dependencies

- [efsw](https://bitbucket.org/SpartanJ/efsw)
- [glew](http://glew.sourceforge.net/)

### Windows

- [etherdream-driver](https://github.com/j4cbo/etherdream-driver)
