#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

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

class GameObject {
public:
	glm::vec3 position;
	glm::mat4 rotation;
	glm::vec3 scale;
	std::shared_ptr<Model> model;

	GameObject(glm::vec3 position, glm::mat4 rotation, glm::vec3 scale, std::string modelName) : position(position), rotation(rotation), scale(scale) {
		std::unordered_map<std::string, std::shared_ptr<Model>>::iterator search = models.find(modelName);
		if (search != models.end())
			model = search->second;
		else {
			std::shared_ptr<Model> m(new Model(FileSystem::getPath("models/" + modelName + "/" + modelName + ".fbx")));
			models.insert({modelName, m});
			model = m;
		}
	}
};
#endif