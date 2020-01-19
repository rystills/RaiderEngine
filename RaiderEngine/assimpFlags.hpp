#pragma once
#include "stdafx.h"
// this file houses the assimp flags used for loading different mesh types
inline unsigned int aiMapProcessFlags =
aiProcess_CalcTangentSpace | // calculate tangents and bitangents if possible
aiProcess_JoinIdenticalVertices | // join identical vertices/ optimize indexing
aiProcess_Triangulate | // Ensure all verticies are triangulated (each 3 vertices are triangle)
aiProcess_ImproveCacheLocality | // improve the cache locality of the output vertices
aiProcess_RemoveRedundantMaterials | // remove redundant materials
aiProcess_FindInvalidData | // detect invalid model data, such as invalid normal vectors
aiProcess_GenUVCoords | // convert spherical, cylindrical, box and planar mapping to proper UVs
aiProcess_TransformUVCoords | // preprocess UV transformations (scaling, translation ...)
aiProcess_FindInstances | // search for instanced meshes and remove them by references to one master
aiProcess_LimitBoneWeights | // limit bone weights to 4 per vertex
aiProcess_OptimizeMeshes | // join small meshes, if possible;
aiProcess_SplitByBoneCount | // split meshes with too many bones. Necessary for our (limited) hardware skinning shader
0;
inline unsigned int aiModelProcessFlags = aiMapProcessFlags | aiProcess_PreTransformVertices; // models should not import with nonstandard transforms; bake the transform instead