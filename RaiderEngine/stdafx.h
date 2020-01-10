#pragma once

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

// external library includes
// glad
#include <glad/glad.h>
// glfw
#include <GLFW/glfw3.h>
#define GL_TEXTURE_MAX_ANISOTROPY_EXT 0x84FE
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF
// glm
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
 // assimp
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
// physx
#define NDEBUG
#include "PxPhysicsAPI.h"
// freetype
#include "ft2build.h"
#include FT_FREETYPE_H
// stb
#include "stb_image.h"
// openalsoft
#include "AL/al.h"
#include "AL/alc.h"
#include "AL/alext.h"