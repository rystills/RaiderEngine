# CMakeList.txt : CMake project for cmakeTemplate, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_STANDARD 23)

# Add source to this project's executable.
#add_executable (RaiderEngine Collider2D.cpp Collider2D.hpp Collider2DCircle.cpp Collider2DCircle.hpp Collider2DLine.cpp Collider2DLine.hpp Collider2DPolygon.cpp Collider2DPolygon.hpp Collider2DRectangle.cpp Collider2DRectangle.hpp FpsDisplay.cpp FpsDisplay.hpp GameObject.cpp GameObject.hpp GameObject2D.cpp GameObject2D.hpp Light.cpp Light.hpp ObjectRegistryBase.cpp ObjectRegistryBase.hpp ParticleEmitter.cpp ParticleEmitter.hpp ParticleEmitter2D.cpp ParticleEmitter2D.hpp PlayerBase.cpp PlayerBase.hpp PlayerSpawn.cpp PlayerSpawn.hpp TextObject.cpp TextObject.hpp Tilemap.cpp Tilemap.hpp alHelpers.cpp alHelpers.hpp assimpFlags.hpp audio.cpp audio.hpp camera.cpp camera.hpp constants.hpp conversionHelpers.cpp conversionHelpers.hpp graphics.cpp graphics.hpp input.cpp input.hpp mapLoader.cpp mapLoader.hpp mesh.cpp mesh.hpp model.cpp model.hpp physics.cpp physics.hpp settings.cpp settings.hpp shader.cpp shader.hpp stb_image.cpp stb_image.h stb_vorbis.c stdafx.cpp stdafx.h terminalColors.hpp timing.cpp timing.hpp)

#target_link_libraries(RaiderEngine glfw glm glad freetype OpenAL assimp PhysXExtensions PhysX PhysXPvdSDK PhysXVehicle PhysXCharacterKinematic PhysXCooking PhysXCommon PhysXFoundation)

# TODO: Add tests and install targets if needed.

add_executable (RaiderEngine main.cpp)
target_link_libraries(RaiderEngine freetype PhysXExtensions PhysX PhysXPvdSDK PhysXVehicle PhysXCharacterKinematic PhysXCooking PhysXCommon PhysXFoundation)