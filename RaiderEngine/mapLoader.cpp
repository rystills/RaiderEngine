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

	// sequentually extract arguments delimited by commas
	std::istringstream tokenStream(name.substr(sParenPos + 1, eParenPos - sParenPos - 1));
	for (std::string token; std::getline(tokenStream, token, ',');) {
		// special named args are stored directly in tempProp
		size_t eqpos = token.find('=');
		if (eqpos == std::string::npos)
			args.push_back(token);
		else {
			std::string namedArg = token.substr(0, eqpos);
			if (namedArg == "castShadows")
				tempCastShadows = token[eqpos+1] == '1';
		}
	}

	return args;
}

void processMapNode(aiNode* node, const aiScene* scene) {
	// determine the full name and real name of the current node
	tempProp.fullName = node->mName.C_Str();
	std::string name = stripNodeName(tempProp.fullName);

	//aiMultiplyMatrix4(&tempProp.trans, &node->mTransformation);
	//aiTransposeMatrix4(&node->mTransformation);
	
	// if we have a parent besides the scene root, multiply our transform by its transform
	if (nodeTransformDict.contains(node->mParent) && strcmp(node->mParent->mName.C_Str(),"RootNode")!=0)
		aiMultiplyMatrix4(&tempProp.trans, &nodeTransformDict[node->mParent]),std::cout << "has parent; parent name is: " << node->mParent->mName.C_Str() << std::endl;

	// check what type of data the current node is designated to store, and update the corresponding transform data if relevant
	bool isTransformNode = false;
	for (int i = 0; i < transformIdentifiers.size(); ++i) {
		if (isTransformNode = tempProp.fullName.find(transformIdentifiers[i]) != std::string::npos) {
			aiMultiplyMatrix4(&tempProp.trans, &node->mTransformation);
			if (i == 0) {
				aiVector3D aiPos, aiRot, aiScale;
				node->mTransformation.Decompose(aiScale, aiRot, aiPos);
				std::cout << "pos: " << aiPos.x << ", " << aiPos.y << ", " << aiPos.z << std::endl;
			}
			if (i == 2) {
				aiVector3D aiPos, aiRot, aiScale;
				node->mTransformation.Decompose(aiScale, aiRot, aiPos);
				std::cout << "geoPos: " << aiPos.x << ", " << aiPos.y << ", " << aiPos.z << std::endl;
			}
			break;
		}
	}

	/*if (node->mName.C_Str() == "rootNode")
		goto clearTransform;*/
	if (!isTransformNode) {
		std::cout << "non-transform node name: " << node->mName.C_Str() << std::endl;
		std::vector<std::string> argList = extractNameArgs(tempProp.fullName);
		// decompose transform for object instantiation
		aiVector3D aiPos, aiRot, aiScale;
		tempProp.trans.Decompose(aiScale, aiRot, aiPos);
		glm::vec3 pos = glm::vec3(aiPos.x, aiPos.y, aiPos.z);
		//std::cout << pos.x << ", " << pos.y << ", " << pos.z << std::endl;
		glm::vec3 rot = glm::vec3(aiRot.x, -aiRot.y, aiRot.z); //glm::vec3(aiRot.x - glm::half_pi<float>(), -aiRot.y, aiRot.z);
		glm::vec3 scale = glm::vec3(aiScale.x, aiScale.y, aiScale.z);

		// convert nodes starting with o_ into GameObject instances using the named model
		if (strncmp(tempProp.fullName.c_str(), "o_", 2) == 0) {
			// load a barebones physics enabled model
			//std::cout << "generating object: " << name << std::endl;
			addGameObject(new GameObject(pos, rot, scale, name))->castShadows &= tempProp.castShadows;
			goto clearTransform;
		}
		else if (strncmp(tempProp.fullName.c_str(), "go_", 3) == 0) {
			// load a class
			//std::cout << "generating instance of GameObject: " << name << std::endl;
			GameObject* go = objectRegistry->instantiateGameObject(name, pos, rot, scale, argList);
			if (go)
				go->castShadows &= tempProp.castShadows;
			goto clearTransform;
		}
		else if (strncmp(tempProp.fullName.c_str(), "l_", 2) == 0) {
			//std::cout << "generating light: " << name << std::endl;
			// create a light
			objectRegistry->instantiateLight(name, pos, rot, scale, argList);
			goto clearTransform;
		}
		else if (node->mNumMeshes > 0) {
			// once we've reached the final node for a static mesh (non-object) process the mesh data and store it as a new model in the scene
			// generate a new model from the mesh list
			//TODO: consider using name here rather than fullName so we can re-use static geometry too
			//std::cout << "generating static geometry: " << tempProp.fullName << std::endl;
			Model* baseModel = new Model();
			//for (unsigned int i = 0; i < node->mNumMeshes; ++i)
				//baseModel->processMesh(scene->mMeshes[node->mMeshes[i]], scene);
			baseModel->generateCollisionShape();
			models.insert(std::make_pair(tempProp.fullName, baseModel));
			addGameObject(new GameObject(pos, rot, scale, tempProp.fullName, true, false, false))->castShadows &= tempProp.castShadows;
			goto clearTransform;
		}
		else {
		clearTransform:
			// copy this transform into our transform dict for group transform lookups
			nodeTransformDict[node] = tempProp.trans;
			// reset the accumulated transform properties first thing once we finish building an object
			tempProp.trans = aiMatrix4x4();
			tempProp.castShadows = true;
		}
	}

	// recurse over child nodes regardless of current node type
	for (unsigned int i = 0; i < node->mNumChildren; ++i)
		processMapNode(node->mChildren[i], scene);
}

void loadMap(std::string mapName) {
	// load the map as a typical model via ASSIMP
	double sTime = glfwGetTime();
	std::string directory = mapDir + '/' + mapName + ".fbx";
	/*Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(directory, aiMapProcessFlags);
	// check for errors
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		ERRORCOLOR(std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl)
		return;
	}*/
	ofbx::IScene* g_scene = nullptr;
	
	#pragma warning(disable:4996)
	FILE* fp = fopen(directory.c_str(), "rb");
	if (!fp) {
		ERRORCOLOR(std::cout << "unable to open map file: '" << directory << "'" << std::endl);
		return;
	}
	std::cout << "Loading map '" << mapName << "'" << std::endl;

	fseek(fp, 0, SEEK_END);
	long file_size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	auto* content = new ofbx::u8[file_size];
	fread(content, 1, file_size, fp);
	g_scene = ofbx::load((ofbx::u8*)content, file_size, (ofbx::u64)ofbx::LoadFlags::TRIANGULATE);
	if (!g_scene) {
		ERRORCOLOR(std::cout << "error while loading map: ";
		OutputDebugString(ofbx::getError()));
		return;
	}
	std::pair<int, int> offsets(0, 0);
	int mesh_count = g_scene->getMeshCount();
	std::cout << "num meshes: " << mesh_count << std::endl;
	for (int i = 0; i < mesh_count; ++i) {
		//std::cout << g_scene->getMesh(i)->name << std::endl;
		const ofbx::Mesh* mesh = g_scene->getMesh(i);
		const char* fullName = mesh->name;
		ofbx::Vec3 opos = mesh->getLocalTranslation();
		ofbx::Vec3 orot = mesh->getLocalRotation();
		ofbx::Vec3 oscale = mesh->getLocalScaling();
		glm::vec3 pos(opos.x, opos.y, opos.z);
		glm::vec3 rot(orot.x - glm::half_pi<float>(), -orot.y, orot.z);
		glm::vec3 scale(oscale.x, oscale.y, oscale.z);
		std::vector<std::string> argList = extractNameArgs(fullName);
		std::string name = stripNodeName(fullName);

		// TODO: name args
		if (strncmp(fullName, "o_", 2) == 0) {
			// load a barebones physics enabled model
			addGameObject(new GameObject(pos, rot, scale, name))->castShadows &= tempCastShadows;
		}
		else if (strncmp(fullName, "go_", 3) == 0) {
			// load a class
			GameObject* go = objectRegistry->instantiateGameObject(name, pos, rot, scale, argList);
			if (go)
				go->castShadows &= tempCastShadows;
		}
		else if (strncmp(fullName, "l_", 2) == 0) {
			// create a light
			objectRegistry->instantiateLight(name, pos, rot, scale, argList);
		}
		else {
			// produce a model from a static mesh
			//TODO: consider using name here rather than fullName so we can re-use static geometry too
			Model* baseModel = new Model();
			baseModel->processMesh(mesh,offsets.first,offsets.second);
			baseModel->generateCollisionShape();
			models.insert(std::make_pair(fullName, baseModel));
			addGameObject(new GameObject(pos, rot, scale, fullName, true, false, false))->castShadows &= tempCastShadows;
		}
		const ofbx::Geometry* geom = mesh->getGeometry();
		offsets.first += geom->getVertexCount();
		offsets.second += geom->getIndexCount();
	}

	delete[] content;
	fclose(fp);

	
	
	// now process nodes recursively with custom instructions since this is a map model
	/*tempProp.prevName = "";
	nodeTransformDict.clear();
	tempProp.trans = aiMatrix4x4();
	processMapNode(scene->mRootNode, scene);*/
	SUCCESSCOLOR(std::cout << "Finished loading map '" << mapName << "' in " << glfwGetTime() - sTime << " seconds" << std::endl)
}