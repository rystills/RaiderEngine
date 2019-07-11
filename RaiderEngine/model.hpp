#pragma once
#include "stdafx.h"
#include "mesh.hpp"
#include "shader.hpp"
#include "assimpFlags.hpp"

std::vector<int> textureFormats = {NULL,GL_RED,NULL,GL_RGB,GL_RGBA};

/*
load the specified texture from the specified directory
@param path: the name of the texture file to load
@returns: the texture id returned by glGenTextures
*/
unsigned int textureFromFile(std::string fileName, bool gamma = false) {
	// generate a new opengl texture to which to write the texture data
	unsigned int textureID;
	glGenTextures(1, &textureID);

	// load the texture data via stbi
	int width, height, nrComponents;
	unsigned char *data = stbi_load(fileName.c_str(), &width, &height, &nrComponents, 0);
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
			ERROR(std::cout << "Error Loading texture '" << fileName << "': invalid number of components '" << nrComponents << "'" << std::endl);
	}
	else
		ERROR(std::cout << "Texture failed to load at path: " << fileName << std::endl);
	
	// cleanup
	stbi_image_free(data);
	return textureID;
}

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
    Model(std::string const &path, bool isStatic = false, bool gamma = false) : isStaticMesh(isStatic), gammaCorrection(gamma) {
        loadModel(path);
		generateCollisionShape();
    }

	/*
	calculate the collision shape for this mesh, to be used by bullet physics
	*/
	void generateCollisionShape() {
		// nothing to generate for an empty mesh
		if (meshes.size() == 0) 
			return;
		// combine verts and tris
		std::vector<Vertex> verts;
		std::vector<unsigned int> inds;
		for (int i = 0; i < meshes.size(); ++i) {
			verts.insert(verts.end(), meshes[i].vertices.begin(), meshes[i].vertices.end());
			inds.insert(inds.end(), meshes[i].indices.begin(), meshes[i].indices.end());
		}
		volume = calculateVolume();
		// note: consider removing the isStaticMesh flag and allowing a Model to contain one or both of the triangle mesh and convex hull depending on the staticness of the GameObjects which use it
		if (isStaticMesh) {
			// create mesh shape from model tris
			PxTriangleMeshDesc meshDesc;
			meshDesc.points.count = verts.size();
			meshDesc.points.stride = sizeof(Vertex);
			meshDesc.points.data = &verts[0];

			// TODO: submeshes may need to be split, as this will create extra triangles between the end of one submesh and the start of another (static meshes probably shouldn't have submeshes anyway?)
			meshDesc.triangles.count = inds.size()/3;
			meshDesc.triangles.stride = 3 * sizeof(unsigned int);
			meshDesc.triangles.data = &inds[0];

			PxDefaultMemoryOutputStream writeBuffer;
			PxTriangleMeshCookingResult::Enum result;
			bool status = gCooking->cookTriangleMesh(meshDesc, writeBuffer, &result);
			if (!status)
				ERROR(std::cout << "error while cooking triangle mesh" << std::endl);

			PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());
			collisionMesh = gPhysics->createTriangleMesh(readBuffer);
		}
		else {
			// create convex hull shape from mesh verts
			PxConvexMeshDesc convexDesc;
			convexDesc.points.count = verts.size();
			convexDesc.points.stride = sizeof(Vertex);
			convexDesc.points.data = &verts[0];
			convexDesc.flags = PxConvexFlag::eCOMPUTE_CONVEX;

			PxDefaultMemoryOutputStream buf;
			if (!gCooking->cookConvexMesh(convexDesc, buf))
				ERROR(std::cout << "error while cooking convex hull" << std::endl);
			
			PxDefaultMemoryInputData readBuffer(buf.getData(), buf.getSize());
			collisionMesh = gPhysics->createConvexMesh(readBuffer);
		}
	}

	/*
	calculate the volume of the current Model's meshes
	@returns: the volume of the current Model's meshes
	*/
	float calculateVolume() {
		float volume = 0;
		for (int j = 0; j < meshes.size(); ++j) {
			Mesh mesh = meshes[j];
			for (int i = 0; i < mesh.indices.size() - 2; i += 3)
				volume += glm::determinant(glm::mat3(mesh.vertices[mesh.indices[i]].Position, mesh.vertices[mesh.indices[i + 1]].Position, mesh.vertices[mesh.indices[i + 2]].Position));
		}
		return volume / 6.0f;  // since the determinant give 6 times tetra volume
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
	@returns: a new Mesh instance containing the relevant ASSIMP mesh data
	*/
	Mesh processMesh(aiMesh *mesh, const aiScene *scene) {
		// data to fill
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;

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

		// return a mesh object created from the extracted mesh data
		return Mesh(vertices, indices, loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse"));
	}

	/*
	load the default map files for each supported map type
	*/
	static void loadDefaultMaterialMaps() {
		Model::defaultDiffuseMap.id = textureFromFile("defaultDiffuseMap.png", ".");
		Model::defaultDiffuseMap.type = "texture_diffuse";
		Model::defaultDiffuseMap.path = "defaultDiffuseMap.png";

		Model::defaultNormalMap.id = textureFromFile("defaultNormalMap.png", ".");
		Model::defaultNormalMap.type = "texture_normal";
		Model::defaultNormalMap.path = "defaultNormalMap.png";

		Model::defaultSpecularMap.id = textureFromFile("defaultSpecularMap.png", ".");
		Model::defaultSpecularMap.type = "texture_specular";
		Model::defaultSpecularMap.path = "defaultSpecularMap.png";

		Model::defaultHeightMap.id = textureFromFile("defaultHeightMap.png", ".");
		Model::defaultHeightMap.type = "texture_height";
		Model::defaultHeightMap.path = "defaultHeightMap.png";
	}
    
private:
    /*
	load the model from the specified path using ASSIMP, and store the resulting meshes in the meshes vector
	*/
    void loadModel(std::string const &path) {
        // read the model file via ASSIMP
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path, aiModelProcessFlags);
        // check for errors
        if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            ERROR(std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl);
            return;
        }
        // retrieve the directory path of the filepath
        std::string directory = path.substr(0, path.find_last_of('/'));

        // process ASSIMP's root node recursively
        processNode(scene->mRootNode, scene);
    }

    /*
	processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
	@param node: the current node to process
	@param scene: the entire scene returned by ASSIMP
	*/
    void processNode(aiNode *node, const aiScene *scene) {
        // process each mesh located at the current node
        for(unsigned int i = 0; i < node->mNumMeshes; ++i)
            // the node object only contains indices to index the actual objects in the scene. 
            // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
            meshes.push_back(processMesh(scene->mMeshes[node->mMeshes[i]], scene));

        // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
        for(unsigned int i = 0; i < node->mNumChildren; ++i)
            processNode(node->mChildren[i], scene);
    }

	/*
	check all material textures of a given type and load the textures if they're not loaded yet
	@param mat: the ASSIMP material
	@param type: the ASSIMP texture type
	@param typeName: a string representation of the texture type (see processMesh for options)
	@returns: a vector containing all of the textures loaded either just now or previously
	*/
   	static std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName) {
		std::vector<Texture> textures;
        for(unsigned int i = 0; i < mat->GetTextureCount(type); ++i) {
            aiString str;
            mat->GetTexture(type, i, &str);
			// manually check for maps other than diffuse rather than specifying them in 3ds max, to simplify workflow a bit
			const int numMapTypes = 4;
			// TODO: don't hardcode png as extension
			std::string mapExtensions[numMapTypes] = { ".png", "_NRM.png", "_SPEC.png", "_DISP.png" };
			std::string mapTypes[numMapTypes] = { "texture_diffuse", "texture_normal", "texture_specular", "texture_height" };
			Texture mapDefaults[numMapTypes] = { defaultDiffuseMap, defaultNormalMap, defaultSpecularMap, defaultHeightMap };
			for (int k = 0; k < numMapTypes; ++k) {
				// get current map name
				std::string mapName = str.C_Str();
				std::string mapBaseName = mapName.substr(0, mapName.find_last_of('.'));
				mapName = mapBaseName + mapExtensions[k];
				
				// check if the current texture has already been loaded
				std::unordered_map<std::string, Texture>::iterator search = texturesLoaded.find(mapName);
				if (search != texturesLoaded.end()) 
					// texture already exists
					textures.push_back(search->second);
				else {
					// texture does not exist yet; try to load it 
					if (std::filesystem::exists(textureDir + mapBaseName + '/' + mapName)) {
						Texture extraTex;
						extraTex.id = textureFromFile(textureDir + mapBaseName + '/' + mapName);
						extraTex.type = mapTypes[k];
						extraTex.path = mapName.c_str();
						textures.push_back(extraTex);
						texturesLoaded[mapName] = extraTex;  // store it as texture loaded for entire model, to ensure we won't unnecesery load duplicate textures.
						SUCCESS(std::cout << "loaded " << mapTypes[k] << " texture: '" << mapName << "'" << std::endl);
					}
					else {
						// can't find texture; fall back to default of matching type
						textures.push_back(mapDefaults[k]);
						if (k) WARNING(std::cout << "unable to find " << mapTypes[k] << " map for texture: '" << str.C_Str() << "'; falling back to default " << mapTypes[k] << " map" << std::endl)
						else ERROR(std::cout << "unable to find diffuse map for texture: '" << str.C_Str() << "'; falling back to default diffuse map" << std::endl);
					}
				}
			}
		}
        return textures;
    }
};
std::unordered_map<std::string, Texture> Model::texturesLoaded;
Texture Model::defaultDiffuseMap, Model::defaultNormalMap, Model::defaultSpecularMap, Model::defaultHeightMap;