# CMakeList.txt : CMake project for cmakeTemplate, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_STANDARD 23)

# Add source to this project's executable.
add_executable (RaiderEngine Collider2D.cpp Collider2D.hpp Collider2DCircle.cpp Collider2DCircle.hpp Collider2DLine.cpp Collider2DLine.hpp Collider2DPolygon.cpp Collider2DPolygon.hpp Collider2DRectangle.cpp Collider2DRectangle.hpp FpsDisplay.cpp FpsDisplay.hpp GameObject.cpp GameObject.hpp GameObject2D.cpp GameObject2D.hpp Light.cpp Light.hpp ObjectRegistryBase.cpp ObjectRegistryBase.hpp ParticleEmitter.cpp ParticleEmitter.hpp ParticleEmitter2D.cpp ParticleEmitter2D.hpp PlayerBase.cpp PlayerBase.hpp PlayerSpawn.cpp PlayerSpawn.hpp TextObject.cpp TextObject.hpp Tilemap.cpp Tilemap.hpp alHelpers.cpp alHelpers.hpp assimpFlags.hpp audio.cpp audio.hpp camera.cpp camera.hpp constants.hpp conversionHelpers.cpp conversionHelpers.hpp graphics.cpp graphics.hpp input.cpp input.hpp mapLoader.cpp mapLoader.hpp mesh.cpp mesh.hpp model.cpp model.hpp physics.cpp physics.hpp settings.cpp settings.hpp shader.cpp shader.hpp stb_image.cpp stb_image.h stb_vorbis.c stdafx.cpp stdafx.h terminalColors.hpp timing.cpp timing.hpp demos/demo3dCarousel/demo3dCarousel.cpp demos/demo3dCarousel/Cog.cpp demos/demo3dCarousel/Cog.hpp demos/demo3dCarousel/Compass.cpp demos/demo3dCarousel/Compass.hpp demos/demo3dCarousel/FlickerLight.cpp demos/demo3dCarousel/FlickerLight.hpp demos/demo3dCarousel/FoliageGrass.cpp demos/demo3dCarousel/FoliageGrass.hpp demos/demo3dCarousel/mouseInteraction.cpp demos/demo3dCarousel/mouseInteraction.hpp demos/demo3dCarousel/ObjectRegistry.cpp demos/demo3dCarousel/ObjectRegistry.hpp)

#target_link_libraries(RaiderEngine glfw glm glad freetype OpenAL assimp PhysXExtensions PhysX PhysXPvdSDK PhysXVehicle PhysXCharacterKinematic PhysXCooking PhysXCommon PhysXFoundation)

# TODO: Add tests and install targets if needed.

#add_executable (RaiderEngine main.cpp)

target_link_libraries(RaiderEngine glad glfw glm OpenAL freetype assimp PhysXExtensions PhysX PhysXPvdSDK PhysXVehicle PhysXCharacterKinematic PhysXCooking PhysXCommon PhysXFoundation)

add_custom_command(TARGET RaiderEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${PROJECT_BINARY_DIR}/RaiderEngine/libs/assimp-5.2.3/bin/assimp-vc142-mtd.dll"
        $<TARGET_FILE_DIR:RaiderEngine>)

#add_custom_command(TARGET RaiderEngine POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E copy_if_different
#        "${PROJECT_BINARY_DIR}/RaiderEngine/libs/freetype-VER-2-12-0/freetyped.dll"
#        $<TARGET_FILE_DIR:RaiderEngine>)