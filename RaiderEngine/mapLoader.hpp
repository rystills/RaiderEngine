#pragma once
#include "stdafx.h"

// this file is responsible for loading maps from special FBX files
struct MapNodeFlags {
	bool usePhysics = true;
	bool castShadows = true;
} inline mapNodeFlags;

inline std::vector<std::string> transformIdentifiers = { "$_Translation", "$_PreRotation", "$_GeometricTranslation", "$_Rotation", "$_Scaling" };

/*
extract the base mesh name from an assimp node name, removing $_transform information, trailing numbers, and constructor arguments
@param fullName: the node name provided by assimp that we wish to strip
@returns: a stripped version of the input node name with trailing transform and numbering info removed
*/
std::string stripNodeName(std::string fullName);

/*
extract all arguments from the specified node name in string form
@param name: the node name from which to extract the arguments
@returns: a vector of extracted string arguments
*/
std::vector<std::string> extractNameArgs(std::string name);

/*
process the current node while loading a map, either extracting a single piece of transform data or finalizing the current object / static mesh
@param node: the node we are currently processing
@param scene: the overall scene returned by ASSIMP when loading the initial map model
*/
void processMapNode(aiNode* node, const aiScene* scene);

/*
load the specified map, instantiating all referenced objects and creating an empty object to house the static geometry
@param mapName: the name of the map to load
*/
void loadMap(std::string mapName);