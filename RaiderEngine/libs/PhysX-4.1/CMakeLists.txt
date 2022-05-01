#project(physx_raiderEngine VERSION 4.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(PHYSX_ROOT_DIR ${CMAKE_SOURCE_DIR}/RaiderEngine/libs/PhysX-4.1/physx)
set(PXSHARED_PATH ${PHYSX_ROOT_DIR}/../pxshared)
set(PXSHARED_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
set(CMAKEMODULES_VERSION "1.27")
set(CMAKEMODULES_PATH ${PHYSX_ROOT_DIR}/../externals/cmakemodules)
set(PX_OUTPUT_LIB_DIR ${CMAKE_CURRENT_BINARY_DIR}/externals/physx)
set(PX_OUTPUT_BIN_DIR ${CMAKE_CURRENT_BINARY_DIR}/externals/physx)

set(PX_BUILDSNIPPETS OFF CACHE BOOL "Generate the snippets" FORCE)
set(PX_BUILDPUBLICSAMPLES OFF CACHE BOOL "Generate the samples projects" FORCE)

set(PX_GENERATE_STATIC_LIBRARIES ON CACHE BOOL "Generate static libraries" FORCE)
set(NV_USE_STATIC_WINCRT OFF CACHE BOOL "Use the statically linked windows CRT" FORCE)
set(NV_USE_DEBUG_WINCRT ON CACHE BOOL "Use the debug version of the CRT" FORCE)

if(WIN32)
    set(TARGET_BUILD_PLATFORM "windows")
else(UNIX)
    if(UNIX AND NOT APPLE)
		set(TARGET_BUILD_PLATFORM "linux")
	else()
		set(TARGET_BUILD_PLATFORM "mac")
	endif()
endif()

#set( PHYSX_LIBRARIES
#	PhysXExtensions
#	PhysX
#	PhysXPvdSDK
#	PhysXVehicle
#	PhysXCharacterKinematic
#	PhysXCooking
#	PhysXCommon
#	PhysXFoundation
#)


add_definitions(-D_DEBUG)

#INCLUDE_DIRECTORIES(${ECHO_ROOT_PATH}/thirdparty/physx/physx/include)
#INCLUDE_DIRECTORIES(${ECHO_ROOT_PATH}/thirdparty/physx/pxshared/include)

#target_include_directories(physx PRIVATE physx/private)
add_subdirectory(physx/compiler/public)