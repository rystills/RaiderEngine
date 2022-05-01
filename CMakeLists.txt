# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)
set(CMAKE_CXX_STANDARD 23)
project (RaiderEngine)

#set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
#set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
#set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
#add_subdirectory(RaiderEngine/libs/glfw-3.3.7)

#add_subdirectory(RaiderEngine/libs/glm-0.9.9.8)

#add_subdirectory(RaiderEngine/libs/glad)

#set(ZLIB_INCLUDE_DIR RaiderEngine/libs/zlib-1.2.12)
#set(ZLIB_LIBRARY RaiderEngine/libs/zlib-1.2.12)
#add_subdirectory(RaiderEngine/libs/zlib-1.2.12)

add_subdirectory(RaiderEngine/libs/freetype-VER-2-12-0)

#add_subdirectory(RaiderEngine/libs/openal-soft-1.22.0)

add_subdirectory(RaiderEngine/libs/assimp-5.2.3)

add_subdirectory(RaiderEngine/libs/PhysX-4.1)

add_subdirectory (RaiderEngine)
