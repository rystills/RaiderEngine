//#include <stdio.h>
//#include <glad/glad.h>
//#include <GLFW/glfw3.h>
//#include <glm/glm.hpp>
//
//#include "AL/al.h"
//#include "AL/alc.h"
//#include "AL/alext.h"
//
//#include <assimp/cimport.h>
//#include <assimp/Importer.hpp>
//#include <assimp/scene.h>
//#include <assimp/postprocess.h>

// c++ standard includes
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <memory>
#include <vector>
#include <string>
#include <cstdlib>
#include <fstream>
#include <map>
#include <filesystem>
#include <utility>
#include <algorithm>
#include <functional>

// physx
#include <PxPhysicsAPI.h>

// freetype
#include <ft2build.h>
#include FT_FREETYPE_H

// assimp
#include <assimp/cimport.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

int main() {
	FT_Library  library;
	int error = FT_Init_FreeType(&library);
	if (error)
	{
		puts("err");
	}


	// declare variables
	physx::PxDefaultAllocator      mDefaultAllocatorCallback;
	physx::PxDefaultErrorCallback  mDefaultErrorCallback;
	physx::PxFoundation* mFoundation = NULL;
	// init physx
	mFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, mDefaultAllocatorCallback, mDefaultErrorCallback);

	Assimp::Importer importer;
	importer.GetErrorString();

	puts("success");
}