project(glad VERSION 0.1.33 LANGUAGES CXX)

add_library(glad include/glad/glad.h src/glad.c)

target_include_directories(glad PUBLIC include/)