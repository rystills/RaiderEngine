#pragma once
#include "stdafx.h"
#include "shader.hpp"

struct Vertex {
    // position
    glm::vec3 Position;
    // normal
    glm::vec3 Normal;
    // texCoords
    glm::vec2 TexCoords;
    // tangent
    glm::vec3 Tangent;
    // bitangent
    glm::vec3 Bitangent;
};

struct Texture {
    unsigned int id;
	std::string type;
	std::string path;
	GLuint width, height;
};

/*
custom deleter for smart pointers containing opengl graphics buffers; deletes the buffer's contents before deleting the buffer itself
@param b: the graphics buffer to delete
*/
void deleteGraphicsBuffer(GLuint* b);

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
	*/
	void sendTexturesToShader(Shader shader);

    // render the mesh
	void draw(Shader shader, bool shouldSendTextures = true);

private:
	std::unique_ptr<GLuint, std::function<void(GLuint*)>> VBO, EBO;

    /*
	initialize all the buffer objects/arrays
	*/
	void setupMesh();
};