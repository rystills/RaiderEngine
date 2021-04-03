#include "stdafx.h"
#include "model.hpp"
#include "mesh.hpp"
#include "shader.hpp"
#include "assimpFlags.hpp"
#include "physics.hpp"
#include "settings.hpp"
#include "terminalColors.hpp"

void textureFromFile(std::string fileName, Texture& texIn, GLuint Wrap_S, GLuint Wrap_T, GLuint Filter_Min, GLuint Filter_Max) {
	// generate a new opengl texture to which to write the texture data
	unsigned int textureID;
	glGenTextures(1, &textureID);

	// load the texture data via stbi (loading the data into memory first does not appear to improve performance vs stbi_load, unlike with stb_vorbis)
	int width, height, nrComponents;
	unsigned char* data = stbi_load(fileName.c_str(), &width, &height, &nrComponents, 0);
	if (data) {
		if (static_cast<unsigned int>(nrComponents) < Model::textureFormats.size() && Model::textureFormats[nrComponents] != NULL) {
			// establish the texture format based on the number of components returned by stbi
			GLenum format = Model::textureFormats[nrComponents];

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
			ERRORCOLOR(std::cout << "Error Loading texture '" << fileName << "': invalid number of components '" << nrComponents << "'" << std::endl);
	}
	else
		ERRORCOLOR(std::cout << "Texture failed to load at path: " << fileName << std::endl);

	// cleanup
	stbi_image_free(data);
	// save to input Texture struct
	texIn.id = textureID;
	texIn.path = fileName;
	texIn.width = width;
	texIn.height = height;
}

Model::Model(std::string const& path, bool isStatic, bool gamma, std::string surfType) : isStaticMesh(isStatic), gammaCorrection(gamma), surfType(surfType) {
	loadModel(path);
	generateCollisionShape();
}

void Model::generateCollisionShape() {
	// nothing to generate for an empty mesh
	if (meshes.size() == 0)
		return;
	// combine vertices
	std::vector<Vertex> verts;
	for (unsigned int i = 0; i < meshes.size(); ++i)
		verts.insert(verts.end(), meshes[i].vertices.begin(), meshes[i].vertices.end());
	volume = calculateVolume();
	// note: consider removing the isStaticMesh flag and allowing a Model to contain one or both of the triangle mesh and convex hull depending on the staticness of the GameObjects which use it
	if (isStaticMesh && surfType == "solid") {
		colliderType = "triangleMesh";
		// combine triangles (vertex lists)
		std::vector<unsigned int> inds;
		for (unsigned int i = 0, vertsAdded = 0; i < meshes.size(); vertsAdded += meshes[i].vertices.size(), ++i) {
			unsigned int curInd = inds.size();
			inds.insert(inds.end(), meshes[i].indices.begin(), meshes[i].indices.end());
			// offset vertex indices by the number of vertices already present in the vector
			if (vertsAdded != 0)
				for (unsigned int r = curInd; r < inds.size(); ++r)
					inds[r] += vertsAdded;
		}
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
			ERRORCOLOR(std::cout << "error while cooking triangle mesh: " << result << std::endl);

		PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());
		collisionMesh = gPhysics->createTriangleMesh(readBuffer);
	}
	else {
		colliderType = "convexHull";
		// create convex hull shape from mesh verts
		PxConvexMeshDesc convexDesc;
		convexDesc.points.count = verts.size();
		convexDesc.points.stride = sizeof(Vertex);
		convexDesc.points.data = &verts[0];
		convexDesc.flags = PxConvexFlag::eCOMPUTE_CONVEX;

		PxDefaultMemoryOutputStream buf;
		if (!gCooking->cookConvexMesh(convexDesc, buf))
			ERRORCOLOR(std::cout << "error while cooking convex hull" << std::endl);

		PxDefaultMemoryInputData readBuffer(buf.getData(), buf.getSize());
		collisionMesh = gPhysics->createConvexMesh(readBuffer);
	}
}

float Model::calculateVolume() {
	float volume = 0;
	for (unsigned int j = 0; j < meshes.size(); ++j) {
		for (unsigned int i = 0; i < meshes[j].indices.size() - 2; i += 3)
			volume += glm::determinant(glm::mat3(meshes[j].vertices[meshes[j].indices[i]].Position,
				meshes[j].vertices[meshes[j].indices[i + 1]].Position, meshes[j].vertices[meshes[j].indices[i + 2]].Position));
	}
	return volume / 6.0f;  // since the determinant give 6 times tetra volume
}

void Model::processMesh(aiMesh* mesh, const aiScene* scene) {
	std::vector<Vertex> vertices(mesh->mNumVertices);
	std::vector<unsigned int> indices;
	// process materials
	// we assume a convention for sampler names in the shaders. Each diffuse texture should be named
	// as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER.
	aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
	std::vector<Texture> matTextures = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");

	// populate vertex data
	for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
		vertices[i].Position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
		vertices[i].Normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
		// a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
		// use models where a vertex can have multiple texture coordinates so we always take the first set (0).
		// NOTE: UVW coordinates must be flipped vertically here; they are then unflipped by the renderer
		vertices[i].TexCoords = mesh->mTextureCoords[0] ? glm::vec4(mesh->mTextureCoords[0][i].x, -mesh->mTextureCoords[0][i].y, matTextures[0].scrollSpeed.x, matTextures[0].scrollSpeed.y) : glm::vec4(0.f);
		vertices[i].Tangent = glm::vec3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);
	}
	// populate index (triangle) data
	for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
		for (unsigned int j = 0; j < mesh->mFaces[i].mNumIndices; ++j)
			indices.push_back(mesh->mFaces[i].mIndices[j]);

	
	// return a mesh object created from the extracted mesh data
	meshes.emplace_back(vertices, indices, matTextures);
}

void Model::createDefaultMaterialMaps() {
	std::vector<std::vector<unsigned char>> mapColors = { {255,0,255},{122,122,255},{122,122,122},{0,0,0},{0,0,0} };
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
		mapDefaults[i]->type = static_cast<MapType>(i);
	}
}

/*
simple texture load meant for 2D sprites
*/
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
		// NOTE: GL_NEAREST resolves artifacts when scaling Tilemaps/atlases and looks sharper, making it ideal for pixel-art. For other styles, the default GL_LINEAR_MIPMAP_LINEAR/GL_LINEAR is likely preferable.
		textureFromFile(textureDir + texFullName, loadedTex, wrapS2D, wrapT2D, filterMin2D, filterMax2D);
		loadedTex.type = texture_diffuse;
		// apply scroll data if present
		std::string scrollMapName = texFullName.substr(0, texFullName.find_last_of('.')) + "_SCROLL.txt";
		if (std::filesystem::exists(textureDir + scrollMapName)) {
			std::ifstream input(textureDir + scrollMapName);
			input >> loadedTex.scrollSpeed.x >> loadedTex.scrollSpeed.y;
		}
		texturesLoaded[texName] = loadedTex;
		SUCCESSCOLOR(std::cout << "loaded texture_diffuse texture: '" << texName << "'" << std::endl);
		return loadedTex;
	}

	ERRORCOLOR(std::cout << "unable to load texture: '" << texName << "' at path: '" << texFullName << "'; falling back to default diffuse map" << std::endl);
	return defaultDiffuseMap;
}

void Model::loadModel(std::string const& path) {
	// read the model file via ASSIMP
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiModelProcessFlags);
	// check for errors
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		ERRORCOLOR(std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl);
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
		processMesh(scene->mMeshes[node->mMeshes[i]], scene);

	// after we've processed all of the meshes (if any) we then recursively process each of the children nodes
	for (unsigned int i = 0; i < node->mNumChildren; ++i)
		processNode(node->mChildren[i], scene);
}

std::vector<Texture> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName) {
	std::vector<Texture> textures;
	for (unsigned int i = 0; i < mat->GetTextureCount(type); ++i) {
		aiString str;
		mat->GetTexture(type, i, &str);

		std::string mapName = str.C_Str();
		int sPos = ignoreModelTexturePaths ? ('\\' + mapName).find_last_of('\\') : 0;
		std::string mapBaseName = mapName.substr(sPos, mapName.find_last_of('.') - sPos);
		// manually check for maps other than diffuse rather than specifying them in 3ds max, to simplify workflow a bit
		for (int k = 0; k < numMapTypes; ++k) {
			// get current map name
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
					extraTex.type = static_cast<MapType>(k);
					// apply scroll data if present
					if (k == 0 && std::filesystem::exists(textureDir + mapBaseName + '/' + mapBaseName + "_SCROLL.txt")) {
						std::ifstream input(textureDir + mapBaseName + '/' + mapBaseName + "_SCROLL.txt");
						input >> extraTex.scrollSpeed.x >> extraTex.scrollSpeed.y;
					}
					textures.push_back(extraTex);
					texturesLoaded[mapName] = extraTex;  // store it as a texture loaded for the entire model, to ensure we won't unnecessarily load duplicate textures
					SUCCESSCOLOR(std::cout << "loaded " << mapTypeNames[k] << " texture: '" << mapName << "'" << std::endl);
				}
				else {
					// can't find texture; fall back to default of matching type
					textures.push_back(*mapDefaults[k]);
					if (k) WARNINGCOLOR(std::cout << "unable to find " << mapTypeNames[k] << " map for texture: '" << str.C_Str() << "'; falling back to default " << mapTypeNames[k] << " map" << std::endl);
					else ERRORCOLOR(std::cout << "unable to find diffuse map for texture: '" << str.C_Str() << "'; falling back to default diffuse map" << std::endl);
				}
			}
		}
	}
	return textures;
}