#ifndef MODEL_H
#define MODEL_H

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
#include <map>
#include <vector>
#include <experimental/filesystem>
std::vector<int> textureFormats = {NULL,GL_RED,NULL,GL_RGB,GL_RGBA};

/*
load the specified texture from the specified directory
@param path: the name of the texture file to load
@param directory: a string containing the directory in which the file resides
@returns: the texture id returned by glGenTextures
*/
unsigned int textureFromFile(const char *path, const std::string &directory, bool gamma = false) {
	// concatenate the file name and directory
	std::string filename = directory + '/' + std::string(path);

	// generate a new opengl texture to which to write the texture data
	unsigned int textureID;
	glGenTextures(1, &textureID);

	// load the texture data via stbi
	int width, height, nrComponents;
	unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
	if (data) {
		if (nrComponents < textureFormats.size() && textureFormats[nrComponents] != NULL) {
			// establish the texture format based on the number of components returned by stbi
			GLenum format = textureFormats[nrComponents];

			// copy the stbi texture data into our new opengl texture
			glBindTexture(GL_TEXTURE_2D, textureID);
			glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);

			// set wrap and magnification filters
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
		else
			std::cout << "Error Loading texture '" << path << "': invalid number of components '" << nrComponents << "'" << std::endl;
	}
	else
		std::cout << "Texture failed to load at path: " << path << std::endl;
	
	// cleanup
	stbi_image_free(data);
	return textureID;
}

class Model {
public:
	static std::vector<Texture> textures_loaded;  // store all textures loaded for re-use across models
	static Texture defaultHeightMap;  // blank heightMap for textures which do not utilize POM
	std::vector<Mesh> meshes;
	bool gammaCorrection;

	/*
	Model default constructor: create a new empty model
	*/
	Model(bool gamma = false) : gammaCorrection(gamma) {};

    /*
	Model constructor: creates a new model with the specified path, optionally performing gamma correction
	@param path: the path to the model we should load
	@param gamma: whether to apply gamma correction (true) or not (false)
	*/
    Model(std::string const &path, bool gamma = false) : gammaCorrection(gamma) {
        loadModel(path);
    }

	/*
	draw the model using the specified shader
	@param shader: the shader to use while drawing the model
	*/
	void draw(Shader shader) {
        for(unsigned int i = 0; i < meshes.size(); ++i)
            meshes[i].draw(shader);
    }

	/*
	process the specified mesh from the scene returned by ASSIMP, producing a mesh instance with the relevant ASSIMP mesh data
	@param mesh: the ASSIMP mesh to process
	@param scene: the scene of which the specified mesh is a part
	@param directory: a string containing the directory in which the file resides
	@returns: a new Mesh instance containing the relevant ASSIMP mesh data
	*/
	Mesh processMesh(aiMesh *mesh, const aiScene *scene, std::string directory) {
		// data to fill
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
		std::vector<Texture> textures;  // TODO: textures should be stored in a hash map for quick lookup during the loading phase

										// Walk through each of the mesh's vertices
		for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
			Vertex vertex;
			glm::vec3 vector; // store data in a temporary glm vector as ASSIMP vectors cannot be directly converted
							  // positions
			vector.x = mesh->mVertices[i].x, vector.y = mesh->mVertices[i].y, vector.z = mesh->mVertices[i].z;
			vertex.Position = vector;
			// normals
			vector.x = mesh->mNormals[i].x, vector.y = mesh->mNormals[i].y, vector.z = mesh->mNormals[i].z;
			vertex.Normal = vector;
			// texture coordinates, if included
			if (mesh->mTextureCoords[0]) {
				glm::vec2 vec;
				// a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
				// use models where a vertex can have multiple texture coordinates so we always take the first set (0).
				vec.x = mesh->mTextureCoords[0][i].x, vec.y = mesh->mTextureCoords[0][i].y;
				vertex.TexCoords = vec;
			}
			else
				vertex.TexCoords = glm::vec2(0.0f, 0.0f);
			// tangent
			vector.x = mesh->mTangents[i].x, vector.y = mesh->mTangents[i].y, vector.z = mesh->mTangents[i].z;
			vertex.Tangent = vector;
			// bitangent
			vector.x = mesh->mBitangents[i].x, vector.y = mesh->mBitangents[i].y, vector.z = mesh->mBitangents[i].z;
			vertex.Bitangent = vector;

			// finished populating the current vertex
			vertices.push_back(vertex);
		}
		// now wak through each of the mesh's faces and retrieve the corresponding vertex indices
		for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
			// retrieve all indices of the face and store them in the indices vector
			for (unsigned int j = 0; j < mesh->mFaces[i].mNumIndices; ++j)
				indices.push_back(mesh->mFaces[i].mIndices[j]);

		// process materials
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		// we assume a convention for sampler names in the shaders. Each diffuse texture should be named
		// as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER. 
		// Same applies to other texture as the following list summarizes:
		// diffuse: texture_diffuseN
		// specular: texture_specularN
		// normal: texture_normalN

		// 1. diffuse maps
		std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse", directory);
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
		// 2. specular maps
		std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular", directory);
		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
		// 3. normal maps
		std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal", directory);
		textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
		// 4. height maps
		std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height", directory);
		textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

		// return a mesh object created from the extracted mesh data
		return Mesh(vertices, indices, textures);
	}
    
private:
    /*
	load the model from the specified path using ASSIMP, and store the resulting meshes in the meshes vector
	*/
    void loadModel(std::string const &path) {
        // read the model file via ASSIMP
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
        // check for errors
        if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
            return;
        }
        // retrieve the directory path of the filepath
        std::string directory = path.substr(0, path.find_last_of('/'));

        // process ASSIMP's root node recursively
        processNode(scene->mRootNode, scene, directory);
    }

    /*
	processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
	@param node: the current node to process
	@param scene: the entire scene returned by ASSIMP
	@param directory: a string containing the directory in which the file resides
	*/
    void processNode(aiNode *node, const aiScene *scene, std::string directory) {
        // process each mesh located at the current node
        for(unsigned int i = 0; i < node->mNumMeshes; ++i)
            // the node object only contains indices to index the actual objects in the scene. 
            // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
            meshes.push_back(processMesh(scene->mMeshes[node->mMeshes[i]], scene, directory));

        // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
        for(unsigned int i = 0; i < node->mNumChildren; ++i)
            processNode(node->mChildren[i], scene, directory);
    }

	/*
	check all material textures of a given type and load the textures if they're not loaded yet
	@param mat: the ASSIMP material
	@param type: the ASSIMP texture type
	@param typeName: a string representation of the texture type (see processMesh for options)
	@param directory: a string containing the directory in which the file resides
	@returns: a vector containing all of the textures loaded either just now or previously
	*/
   	static std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName, std::string directory) {
		std::vector<Texture> textures;
        for(unsigned int i = 0; i < mat->GetTextureCount(type); ++i) {
            aiString str;
            mat->GetTexture(type, i, &str);
            // check all loaded textures to see if the current texture was loaded before
            bool skip = false;
            for(unsigned int j = 0; j < textures_loaded.size(); j++) {
                if(std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0) {
                    textures.push_back(textures_loaded[j]);
                    skip = true;
                    break;
                }
            }
			// the texture hasn't been loaded before so load it now
            if(!skip) {
                Texture texture;
                texture.id = textureFromFile(str.C_Str(), directory);
                texture.type = typeName;
                texture.path = str.C_Str();
                textures.push_back(texture);
                textures_loaded.push_back(texture);  // store it as texture loaded for entire model, to ensure we won't unnecesery load duplicate textures.
				std::cout << "loaded file: '" << str.C_Str() << "'" << std::endl;

				// TODO: currently we manually check for maps other than diffuse due to collada export issues; down the line this should become unnecessary
				std::string normalName = str.C_Str();
				// TODO: don't hardcode png as extension
				normalName = normalName.substr(0, normalName.find_last_of('.')) + "_NRM.png";
				if (std::experimental::filesystem::exists(directory + "/" + normalName)) {
					Texture textureNrm;
					textureNrm.id = textureFromFile(normalName.c_str(), directory);
					textureNrm.type = "texture_normal";
					textureNrm.path = normalName.c_str();
					textures.push_back(textureNrm);
					textures_loaded.push_back(textureNrm);  // store it as texture loaded for entire model, to ensure we won't unnecesery load duplicate textures.
					std::cout << "loaded file: '" << normalName << "'" << std::endl;
				}
				std::string specName = str.C_Str();
				specName = specName.substr(0, specName.find_last_of('.')) + "_SPEC.png";
				if (std::experimental::filesystem::exists(directory + "/" + specName)) {
					Texture textureSpec;
					textureSpec.id = textureFromFile(specName.c_str(), directory);
					textureSpec.type = "texture_specular";
					textureSpec.path = specName.c_str();
					textures.push_back(textureSpec);
					textures_loaded.push_back(textureSpec);  // store it as texture loaded for entire model, to ensure we won't unnecesery load duplicate textures.
					std::cout << "loaded file: '" << specName << "'" << std::endl;
				}
				std::string heightName = str.C_Str();
				heightName = heightName.substr(0, heightName.find_last_of('.')) + "_DISP.png";
				if (std::experimental::filesystem::exists(directory + "/" + heightName)) {
					Texture textureHeight;
					textureHeight.id = textureFromFile(heightName.c_str(), directory);
					textureHeight.type = "texture_height";
					textureHeight.path = heightName.c_str();
					textures.push_back(textureHeight);
					textures_loaded.push_back(textureHeight);  // store it as texture loaded for entire model, to ensure we won't unnecesery load duplicate textures.
					std::cout << "loaded file: '" << heightName << "'" << std::endl;
				}
				else {
					textures.push_back(defaultHeightMap);
					std::cout << "unable to find heightMap for texture: '" << str.C_Str() << "'; falling back to default heightMap" << std::endl;
				}
            }
        }
        return textures;
    }
};
std::vector<Texture> Model::textures_loaded;
Texture Model::defaultHeightMap;
#endif