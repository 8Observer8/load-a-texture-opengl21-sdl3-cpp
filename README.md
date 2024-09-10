[Setting up SDL3 for Windows using CMake and MinGW-w64](https://8observer8.github.io/tutorials/setting-up/sdl3-for-windows/public/index.html)

> mkdir dist\win

> cd dist

> cmake -G "MinGW Makefiles" --fresh -S .. -B ..\dist\win -DSDL3_DIR=E:\libs\sdl3-desktop-prefix\lib\cmake\SDL3

> cd win

> mingw32-make

> app
