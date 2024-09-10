cmake_minimum_required(VERSION 3.20)

project(load_a_texture)
add_executable(app WIN32)

target_include_directories(app PRIVATE E:/libs/glad-2.0.6/include)
target_include_directories(app PRIVATE E:/libs/stb_image-2.27/include)

target_sources(app
PRIVATE
    E:/libs/glad-2.0.6/src/glad.c
    main.cpp
)

# This code copies the "SDL3.dll" file to the build directory
add_custom_command(TARGET app POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_RUNTIME_DLLS:app> $<TARGET_FILE_DIR:app>
)

find_package(SDL3)
target_link_libraries(app PRIVATE SDL3::SDL3)

target_link_options(app PRIVATE -static-libgcc -static-libstdc++)
target_compile_definitions(app PRIVATE SDL_MAIN_USE_CALLBACKS)

# C++ 20 requires for the "battery-embed" library
target_compile_features(app PUBLIC cxx_std_20)

# This code will install the "battery-embed" library
include(FetchContent)
FetchContent_Declare(
  battery-embed
  GIT_REPOSITORY https://github.com/batterycenter/embed.git
  GIT_TAG        main
)
FetchContent_MakeAvailable(battery-embed)

# Files to embed
b_embed(app assets/crate.png)
