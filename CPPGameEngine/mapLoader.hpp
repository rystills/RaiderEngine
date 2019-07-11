#include "stdafx.h"
#pragma once
#include "assimpFlags.hpp"
#include "ObjectRegistry.hpp"
#include "settings.hpp"

// this file is responsible for loading maps from special FBX files

struct ProcessObjectProperties {
	glm::vec3 pos, rot, scale, geoPos;
	std::string fullName, prevName;
} tempProp;
/*
extract the base mesh name from an assimp node name, removing $_transform information, trailing numbers, and constructor arguments
@param fullName: the node name provided by assimp that we wish to strip
@returns: a stripped version of the input node name with trailing transform and numbering info removed
*/
std::string stripNodeName(std::string fullName) {
	std::size_t nameExtraStart = fullName.find("_$Assimp");
	std::size_t underscorePos = fullName.find("_");
	// fund underscore position, ignoring underscores added in by ASSIMP
	int sPos = (underscorePos == std::string::npos || (underscorePos != fullName.length() - 1 && fullName[underscorePos + 1] == '$') ? 0 : underscorePos + 1);
	std::string name = nameExtraStart == std::string::npos ? fullName.substr(sPos) : fullName.substr(sPos, nameExtraStart - sPos);
	// strip trailing numbers applied to duplicate object names in the newest version of assimp
	while (isdigit(name[name.length() - 1]))
		name = name.substr(0, name.length() - 1);

	// strip constructor arguments
	if (name[name.length() - 1] == ')')
		name = name.substr(0, name.find("("));

	return name;
}

/*
extract all arguments from the specified node name in string form
@param name: the node name from which to extract the arguments
@returns: a vector of extracted string arguments
*/
std::vector<std::string> extractNameArgs(std::string name) {
	std::vector<std::string> args;
	// find the start and end of the argument list
	std::size_t sParenPos = name.find("(");
	std::size_t eParenPos = name.find(")");
	if (sParenPos == std::string::npos || eParenPos == std::string::npos)
		return args;

	// sequentually extract arguments delimited by commas
	std::istringstream tokenStream(name.substr(sParenPos + 1, eParenPos - sParenPos - 1));
	for (std::string token; std::getline(tokenStream, token, ','); args.push_back(token));

	return args;
}


// TODO: do something more elegant than using a global temp struct
/*
process the current node while loading a map, either extracting a single piece of transform data or finalizing the current object / static mesh
@param node: the node we are currently processing
@param scene: the overall scene returned by ASSIMP when loading the initial map model
*/
void processMapNode(aiNode *node, const aiScene *scene) {
	// determine the full name and real name of the current node
	tempProp.fullName = node->mName.C_Str();
	std::string name = stripNodeName(tempProp.fullName);

	// extract the transform data from the current node
	aiVector3D aiPos, aiRot, aiScale;
	node->mTransformation.Decompose(aiScale, aiRot, aiPos);
	glm::vec3 pos = glm::vec3(aiPos.x, aiPos.y, aiPos.z);
	glm::vec3 rot = glm::vec3(aiRot.x - glm::half_pi<float>(), -aiRot.y, aiRot.z);
	glm::vec3 scale = glm::vec3(aiScale.x, aiScale.y, aiScale.z);

	// check what type of data the current node is designated to store, and update the corresponding transform data if relevant
	bool isTransformNode = false;
	if (tempProp.fullName.find("$_Translation") != std::string::npos) {
		tempProp.pos = pos;
		isTransformNode = true;
	}
	else if (tempProp.fullName.find("$_PreRotation") != std::string::npos) {
		// note: pre-rotation data is ignored and manually applied to rotation above
		isTransformNode = true;
	}
	else if (tempProp.fullName.find("$_GeometricTranslation") != std::string::npos) {
		isTransformNode = true;
		tempProp.geoPos = pos;
	}
	else if (tempProp.fullName.find("$_Rotation") != std::string::npos) {
		tempProp.rot = rot;
		isTransformNode = true;
	}
	else if (tempProp.fullName.find("$_Scaling") != std::string::npos) {
		tempProp.scale = scale;
		isTransformNode = true;
	}

	if (!isTransformNode) {
		std::vector<std::string> argList = extractNameArgs(tempProp.fullName);
		// convert nodes starting with o_ into GameObject instances using the named model
		if (strncmp(tempProp.fullName.c_str(), "o_", 2) == 0) {
			// load a barebones physics enabled model
			//std::cout << "generating object: " << name << std::endl;
			gameObjects.emplace_back(new GameObject(tempProp.pos + tempProp.geoPos, tempProp.rot, tempProp.scale, name));
			goto clearTransform;
		}
		else if (strncmp(tempProp.fullName.c_str(), "go_", 3) == 0) {
			// load a class
			//std::cout << "generating instance of GameObject: " << name << std::endl;
			instantiateGameObject(name, tempProp.pos + tempProp.geoPos, tempProp.rot, tempProp.scale, argList);
			goto clearTransform;
		}
		else if (strncmp(tempProp.fullName.c_str(), "l_", 2) == 0) {
			//std::cout << "generating light: " << name << std::endl;
			// create a light
			instantiateLight(name, tempProp.pos + tempProp.geoPos, tempProp.rot, tempProp.scale, argList);
			goto clearTransform;
		}
		else if (node->mNumMeshes > 0) {
			// once we've reached the final node for a static mesh (non-object) process the mesh data and store it as a new model in the scene
			// generate a new model from the mesh list
			//TODO: consider using name here rather than fullName so we can re-use static geometry too
			//std::cout << "generating static geometry: " << tempProp.fullName << std::endl;
			std::shared_ptr<Model> baseModel = std::make_shared<Model>();
			for (unsigned int i = 0; i < node->mNumMeshes; ++i)
				baseModel->meshes.push_back(baseModel->processMesh(scene->mMeshes[node->mMeshes[i]], scene));
			baseModel->generateCollisionShape();
			models.insert({ tempProp.fullName, baseModel });
			gameObjects.emplace_back(new GameObject(tempProp.pos + tempProp.geoPos, glm::vec3(tempProp.rot.x, tempProp.rot.y, tempProp.rot.z), tempProp.scale, tempProp.fullName, true, false, false));
			goto clearTransform;
		}
		else {
		clearTransform:
			// reset the accumulated transform properties first thing once we finish building an object
			tempProp.pos = glm::vec3(0, 0, 0);
			tempProp.geoPos = glm::vec3(0, 0, 0);
			tempProp.rot = glm::vec3(-glm::half_pi<float>(), 0, 0);
			tempProp.scale = glm::vec3(1, 1, 1);
		}
	}

	// recurse over child nodes regardless of current node type
	for (unsigned int i = 0; i < node->mNumChildren; ++i)
		processMapNode(node->mChildren[i], scene);
}
/*
load the specified map, instantiating all referenced objects and creating an empty object to house the static geometry
@param mapName: the name of the map to load
*/
void loadMap(std::string mapName) {
	// TODO: don't use hard-coded map folder
	// load the map as a typical model via ASSIMP
	std::string directory = FileSystem::getPath(mapDir + '/' + mapName + ".fbx");
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(directory, aiMapProcessFlags);
	// check for errors
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		ERROR(std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl);
		return;
	}
	std::cout << "Loading map '" << mapName << "'" << std::endl;
	// now process nodes recursively with custom instructions since this is a map model
	tempProp.prevName = "";
	processMapNode(scene->mRootNode, scene);
	SUCCESS(std::cout << "Finished loading map '" << mapName << "'" << std::endl);
}