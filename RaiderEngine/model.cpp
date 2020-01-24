#include "stdafx.h"
#include "model.hpp"
#include "mesh.hpp"
#include "shader.hpp"
#include "assimpFlags.hpp"
#include "physics.hpp"
#include "settings.hpp"
#include "terminalColors.hpp"

std::unordered_map<std::string, Texture> Model::texturesLoaded;
Texture Model::defaultDiffuseMap, Model::defaultNormalMap, Model::defaultSpecularMap, Model::defaultHeightMap;

void textureFromFile(std::string fileName, Texture& texIn, GLuint Wrap_S, GLuint Wrap_T, GLuint Filter_Min, GLuint Filter_Max) {
	// generate a new opengl texture to which to write the texture data
	unsigned int textureID;
	glGenTextures(1, &textureID);

	// load the texture data via stbi
	int width, height, nrComponents;
	unsigned char* data = stbi_load(fileName.c_str(), &width, &height, &nrComponents, 0);
	if (data) {
		if (nrComponents < textureFormats.size() && textureFormats[nrComponents] != NULL) {
			// establish the texture format based on the number of components returned by stbi
			GLenum format = textureFormats[nrComponents];

			// copy the stbi texture data into our new opengl texture
			glBindTexture(GL_TEXTURE_2D, textureID);
			glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);

			// set wrap and magnification filters
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, Wrap_S);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, Wrap_T);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, Filter_Min);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, Filter_Max);
		}
		else
			ERROR(std::cout << "Error Loading texture '" << fileName << "': invalid number of components '" << nrComponents << "'" << std::endl);
	}
	else
		ERROR(std::cout << "Texture failed to load at path: " << fileName << std::endl);

	// cleanup
	stbi_image_free(data);
	// save to input Texture struct
	texIn.id = textureID;
	texIn.path = fileName;
	texIn.width = width;
	texIn.height = height;
}

Model::Model(std::string const& path, bool isStatic, bool gamma) : isStaticMesh(isStatic), gammaCorrection(gamma) {
	loadModel(path);
	generateCollisionShape();
}

void Model::generateCollisionShape() {
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
		meshDesc.triangles.count = inds.size() / 3;
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

float Model::calculateVolume() {
	float volume = 0;
	for (int j = 0; j < meshes.size(); ++j) {
		for (int i = 0; i < meshes[j].indices.size() - 2; i += 3)
			volume += glm::determinant(glm::mat3(meshes[j].vertices[meshes[j].indices[i]].Position,
				meshes[j].vertices[meshes[j].indices[i + 1]].Position, meshes[j].vertices[meshes[j].indices[i + 2]].Position));
	}
	return volume / 6.0f;  // since the determinant give 6 times tetra volume
}

void Model::draw(Shader shader, bool shouldSendTextures) {
	for (unsigned int i = 0; i < meshes.size(); ++i)
		meshes[i].draw(shader, shouldSendTextures);
}

Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene) {
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

void Model::createDefaultMaterialMaps() {
	Texture* mapDefaults[numMapTypes] = { &Model::defaultDiffuseMap, &Model::defaultNormalMap, &Model::defaultSpecularMap, &Model::defaultHeightMap };
	std::vector<std::vector<unsigned char>> mapColors = { {255,0,255},{122,122,255},{122,122,122},{0,0,0} };
	for (int i = 0; i < numMapTypes; ++i) {
		// setup a new texture
		unsigned int textureID;
		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);

		// copy in our texture data
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, &mapColors[i][0]);
		glGenerateMipmap(GL_TEXTURE_2D);

		// set texture flags
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// save to input Texture struct
		mapDefaults[i]->id = textureID;
		mapDefaults[i]->path = "";
		mapDefaults[i]->width = 1;
		mapDefaults[i]->height = 1;
		mapDefaults[i]->type = mapTypes[i];
	}
}

Texture Model::loadTextureSimple(std::string texFullName) {
	// check if the current texture has already been loaded
	std::string texName = texFullName.substr(texFullName.find_last_of('/') + 1);
	std::unordered_map<std::string, Texture>::iterator search = texturesLoaded.find(texName);
	if (search != texturesLoaded.end())
		// texture already exists
		return search->second;

	// texture does not exist yet; try to load it 
	if (std::filesystem::exists(textureDir + texFullName)) {
		Texture loadedTex;
		textureFromFile(textureDir + texFullName, loadedTex);
		loadedTex.type = "texture_diffuse";
		texturesLoaded[texName] = loadedTex;
		SUCCESS(std::cout << "loaded texture_diffuse texture: '" << texName << "'" << std::endl);
		return loadedTex;
	}

	ERROR(std::cout << "unable to load texture: '" << texName << "' at path: '" << texFullName << "'; falling back to default diffuse map" << std::endl);
	return defaultDiffuseMap;
}

void Model::loadModel(std::string const& path) {
	// read the model file via ASSIMP
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiModelProcessFlags);
	// check for errors
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		ERROR(std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl);
		return;
	}

	// process ASSIMP's root node recursively
	processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode* node, const aiScene* scene) {
	// process each mesh located at the current node
	for (unsigned int i = 0; i < node->mNumMeshes; ++i)
		// the node object only contains indices to index the actual objects in the scene. 
		// the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
		meshes.push_back(processMesh(scene->mMeshes[node->mMeshes[i]], scene));

	// after we've processed all of the meshes (if any) we then recursively process each of the children nodes
	for (unsigned int i = 0; i < node->mNumChildren; ++i)
		processNode(node->mChildren[i], scene);
}

std::vector<Texture> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName) {
	std::vector<Texture> textures;
	for (unsigned int i = 0; i < mat->GetTextureCount(type); ++i) {
		aiString str;
		mat->GetTexture(type, i, &str);
		// manually check for maps other than diffuse rather than specifying them in 3ds max, to simplify workflow a bit
		// TODO: don't hardcode png as extension
		std::string mapExtensions[numMapTypes] = { ".png", "_NRM.png", "_SPEC.png", "_DISP.png" };
		Texture* mapDefaults[numMapTypes] = { &Model::defaultDiffuseMap, &Model::defaultNormalMap, &Model::defaultSpecularMap, &Model::defaultHeightMap };
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
					textureFromFile(textureDir + mapBaseName + '/' + mapName, extraTex);
					extraTex.type = mapTypes[k];
					textures.push_back(extraTex);
					texturesLoaded[mapName] = extraTex;  // store it as texture loaded for entire model, to ensure we won't unnecesery load duplicate textures.
					SUCCESS(std::cout << "loaded " << mapTypes[k] << " texture: '" << mapName << "'" << std::endl);
				}
				else {
					// can't find texture; fall back to default of matching type
					textures.push_back(*mapDefaults[k]);
					if (k) WARNING(std::cout << "unable to find " << mapTypes[k] << " map for texture: '" << str.C_Str() << "'; falling back to default " << mapTypes[k] << " map" << std::endl)
					else ERROR(std::cout << "unable to find diffuse map for texture: '" << str.C_Str() << "'; falling back to default diffuse map" << std::endl);
				}
			}
		}
	}
	return textures;
}