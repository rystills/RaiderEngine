#ifndef LIGHT_H
#define LIGHT_H

#include <glad/glad.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "mesh.h"
#include "shader.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <memory>

class Light {
public:
	glm::vec3 position, color;
	float linear, quadratic;
	Light(glm::vec3 position, glm::vec3 color, float linear = 0.045f, float quadratic = .0075f) : position(position), color(color), linear(linear), quadratic(quadratic) {}
};
#endif