#include "stdafx.h"
#include "assimpFlags.hpp"
#include "ObjectRegistryBase.hpp"
#include "settings.hpp"
#include "mapLoader.hpp"
#include "terminalColors.hpp"

std::string stripNodeName(std::string fullName) {
	std::size_t nameExtraStart = fullName.find("_$Assimp");
	std::size_t underscorePos = fullName.find("_");
	// find underscore position, ignoring underscores added in by ASSIMP
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

std::vector<std::string> extractNameArgs(std::string name) {
	std::vector<std::string> args;
	// find the start and end of the argument list
	std::size_t sParenPos = name.find("(");
	std::size_t eParenPos = name.find(")");
	if (sParenPos == std::string::npos || eParenPos == std::string::npos)
		return args;

	// sequentially extract arguments delimited by commas
	std::istringstream tokenStream(name.substr(sParenPos + 1, eParenPos - sParenPos - 1));
	for (std::string token; std::getline(tokenStream, token, ',');) {
		// special named args are stored in mapNodeFlags
		size_t eqpos = token.find('=');
		if (eqpos == std::string::npos)
			args.push_back(token);
		else {
			std::string namedArg = token.substr(0, eqpos);
			if (namedArg == "castShadows")
				mapNodeFlags.castShadows = token[eqpos + 1] == '1';
			else if (namedArg == "usePhysics")
				mapNodeFlags.usePhysics = token[eqpos + 1] == '1';
			else if (namedArg == "enableShadows")
				mapNodeFlags.enableShadows = token[eqpos + 1] == '1';
			else if (namedArg == "surfType")
				mapNodeFlags.surfType = &token[eqpos + 1];
			else if (namedArg == "drawTwoSided")
				mapNodeFlags.drawTwoSided = &token[eqpos + 1];
		}
	}
	return args;
}

void processMapNode(aiNode* node, const aiScene* scene) {
	std::string fullName = node->mName.C_Str();
	// check if the current node contains a transform
	bool isTransformNode = false;
	for (int i = 0; i < transformIdentifiers.size() && !isTransformNode; ++i)
		isTransformNode = fullName.find(transformIdentifiers[i]) != std::string::npos;

	if (!isTransformNode) {
		std::string name = stripNodeName(fullName);
		std::vector<std::string> argList = extractNameArgs(fullName);
		// calculate transform by multiplying transform matrices from the parent node all the way down to the current node
		std::vector<aiNode*> nodes;
		for (aiNode* n = node; n != NULL; n = n->mParent)
			nodes.push_back(n);

		aiMatrix4x4 trans = nodes[nodes.size()-1]->mTransformation;
		nodes.pop_back();
		while (!nodes.empty()) {
			trans *= nodes[nodes.size() - 1]->mTransformation;
			nodes.pop_back();
		}
		
		// decompose transform for object instantiation
		aiVector3D aiPos, aiRot, aiScale;
		trans.Decompose(aiScale, aiRot, aiPos);
		glm::vec3 pos = glm::vec3(aiPos.x, aiPos.y, aiPos.z);
		glm::vec3 rot = glm::vec3(aiRot.x, aiRot.y, aiRot.z);
		glm::vec3 scale = glm::vec3(aiScale.x, aiScale.y, aiScale.z);
		// NOTE: changing the pivot of a 'go_' / 'l_' mesh in the map file won't have any effect in assimp unless you reset xform afterwards;
		// if you want to change the actual object's pivot rather than just the spawn location, you'll need to offset its mesh in the corresponding model file
		if (strncmp(fullName.c_str(), "go_", 3) == 0)
			// convert a node starting with go_ into a specific GameObject instance defined in the object registry
			// NOTE: MapNodeFlags are applied to generic models by default, but must be explicitly handled when overloading ObjectRegistryBase to spawn custom classes
			// NOTE: models loaded in from assimp are offset by 90 degrees (pi/2); this cancels out for static meshes and manual instantiation, but must be corrected here for GameObjects referencing their own models
			objectRegistry->instantiateGameObject(name, pos, rot + glm::vec3(glm::half_pi<float>(),0,0), scale, argList);
		else if (strncmp(fullName.c_str(), "l_", 2) == 0)
			// convert a node starting with l_ into a specific Light instance defined in the object registry
			objectRegistry->instantiateLight(name, pos, rot, scale, argList);
		else if (node->mNumMeshes > 0) {
			// once we've reached the final node for a static mesh (non-object) process the mesh data and store it as a new model in the scene
			// generate a new model from the mesh list
			Model* baseModel = new Model();
			baseModel->surfType = mapNodeFlags.surfType;
			for (unsigned int i = 0; i < node->mNumMeshes; ++i)
				baseModel->processMesh(scene->mMeshes[node->mMeshes[i]], scene);
			baseModel->generateCollisionShape();
			models.insert(std::make_pair(fullName, baseModel));
			addGameObject(new GameObject(pos, rot, scale, fullName, true, false, mapNodeFlags.usePhysics, mapNodeFlags.castShadows, mapNodeFlags.drawTwoSided));
		}
		// reset the current node flags
		mapNodeFlags.surfType = "solid";
		mapNodeFlags.usePhysics = true;
		mapNodeFlags.castShadows = true;
		mapNodeFlags.enableShadows = true;
		mapNodeFlags.drawTwoSided = false;
	}

	// recurse over child nodes regardless of current node type
	for (unsigned int i = 0; i < node->mNumChildren; ++i)
		processMapNode(node->mChildren[i], scene);
}

void loadMap(std::string mapName) {
	// load the map as a typical model via ASSIMP
	double sTime = glfwGetTime();
	std::string directory = mapDir + '/' + mapName + ".fbx";
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(directory, aiMapProcessFlags);
	// check for errors
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		ERRORCOLOR(std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl);
		return;
	}
	std::cout << "Loading map '" << mapName << "'" << std::endl;
	// now process nodes recursively with custom instructions since this is a map model
	processMapNode(scene->mRootNode, scene);
	SUCCESSCOLOR(std::cout << "Finished loading map '" << mapName << "' in " << glfwGetTime() - sTime << " seconds" << std::endl);
}