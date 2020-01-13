#pragma once
#include "stdafx.h"
#include "mesh.hpp"
#include "physics.hpp"

inline std::vector<int> textureFormats = {NULL,GL_RED,NULL,GL_RGB,GL_RGBA};
inline const int numMapTypes = 4;
inline std::string mapTypes[numMapTypes] = { "texture_diffuse", "texture_normal", "texture_specular", "texture_height" };

/*
load the specified texture from the specified directory
@param path: the name of the texture file to load
@returns: the texture id returned by glGenTextures
*/
void textureFromFile(std::string fileName, Texture& texIn, GLuint Wrap_S = GL_REPEAT, GLuint Wrap_T = GL_REPEAT, GLuint Filter_Min = GL_LINEAR_MIPMAP_LINEAR, GLuint Filter_Max = GL_LINEAR);

class Model {
public:
	static std::unordered_map<std::string, Texture> texturesLoaded;  // store all textures loaded for re-use across models 
	static Texture defaultDiffuseMap, defaultNormalMap, defaultSpecularMap, defaultHeightMap;  // blank maps for materials which don't use the given effects
	std::vector<Mesh> meshes;
	bool gammaCorrection;
	PxBase* collisionMesh;
	bool isStaticMesh;
	float volume;

	/*
	Model default constructor: create a new empty model
	*/
	Model(bool isStatic = true, bool gamma = false) : isStaticMesh(isStatic), gammaCorrection(gamma) {};

    /*
	Model constructor: creates a new model with the specified path, optionally performing gamma correction
	@param path: the path to the model we should load
	@param isStatic: whether this is a static model (and therefore can use a triangle mesh) or a dynamic model (and therefore should default to a convex hull)
	@param gamma: whether to apply gamma correction (true) or not (false)
	*/
	Model(std::string const& path, bool isStatic = false, bool gamma = false);

	/*
	calculate the collision shape for this mesh, to be used by bullet physics
	*/
	void generateCollisionShape();

	/*
	calculate the volume of the current Model's meshes
	@returns: the volume of the current Model's meshes
	*/
	float calculateVolume();

	/*
	draw the model using the specified shader
	@param shader: the shader to use while drawing the model
	@param shouldSendTextures: whether or not to bind the appropriate textures before rendering
	*/
	void draw(Shader shader, bool shouldSendTextures = true);

	/*
	process the specified mesh from the scene returned by ASSIMP, producing a mesh instance with the relevant ASSIMP mesh data
	@param mesh: the ASSIMP mesh to process
	@param scene: the scene of which the specified mesh is a part
	@returns: a new Mesh instance containing the relevant ASSIMP mesh data
	*/
	Mesh processMesh(aiMesh* mesh, const aiScene* scene);

	/*
	generate the default material maps, used as fallbacks when the respective map type is not present for a texture
	*/
	static void createDefaultMaterialMaps();

	/*
	load a single diffuse texture file if it does not already exist
	*/
	static Texture loadTextureSimple(std::string texFullName);
    
private:
    /*
	load the model from the specified path using ASSIMP, and store the resulting meshes in the meshes vector
	*/
	void loadModel(std::string const& path);

    /*
	processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
	@param node: the current node to process
	@param scene: the entire scene returned by ASSIMP
	*/
	void processNode(aiNode* node, const aiScene* scene);

	/*
	check all material textures of a given type and load the textures if they're not loaded yet
	@param mat: the ASSIMP material
	@param type: the ASSIMP texture type
	@param typeName: a string representation of the texture type (see processMesh for options)
	@returns: a vector containing all of the textures loaded either just now or previously
	*/
	static std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);
};