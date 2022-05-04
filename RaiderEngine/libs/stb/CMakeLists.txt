project(stb)

add_library(stb STATIC ./stb_vorbis.c ./stb_image.cpp)

target_include_directories(stb PUBLIC ./)