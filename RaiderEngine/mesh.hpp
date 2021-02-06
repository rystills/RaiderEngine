#pragma once
#include "stdafx.h"
#include "shader.hpp"

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec4 TexCoords;
    glm::vec3 Tangent;
};

struct Texture {
    unsigned int id;
	enum MapType type;
	std::string path;
	GLuint width, height;
	glm::vec2 scrollSpeed = glm::vec2(0);
};

/*
custom deleter for smart pointers containing VAOs; deletes the VAO's contents before deleting the pointer
@param v: pointer to the VAO to delete
*/
void deleteVAO(GLuint* v);

class Mesh {
public:
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> textures;
	std::unique_ptr<GLuint, std::function<void(GLuint*)>> VAO;

	/*
	Mesh constructor: creates a new mesh with the provided vertex, index, and texture data
	@param vertices: a vector of vertices that comprise the mesh
	@param indices: a vector of indices that comprise the mesh
	@param textures: a vector of textures that the mesh uses
	*/
	Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);

	/*
	bind all of the textures used by this mesh for rendering, and update the shader uniforms accordingly
	@param shader: the shader to which we will send our textures
	@param diffuseOnly: whether we will send only the diffuse texture, or all textures
	*/
	void sendTexturesToShader(Shader shader, bool diffuseOnly = false);

private:
    /*
	initialize all the buffer objects/arrays
	*/
	void setupMesh();
};