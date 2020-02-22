#include "stdafx.h"
#include "shader.hpp"
#include "mesh.hpp"

void deleteVAO(GLuint* v) {
	glDeleteVertexArrays(1, v);
	delete v;
}

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures) {
	this->vertices = vertices;
	this->indices = indices;
	this->textures = textures;

	// now that we have all the required data, set the vertex buffers and its attribute pointers.
	setupMesh();
}

void Mesh::sendTexturesToShader(Shader shader) {
	// bind appropriate textures
	unsigned int diffuseNr = 0, specularNr = 0, normalNr = 0, heightNr = 0;
	for (unsigned int i = 0; i < textures.size(); i++) {
		glActiveTexture(GL_TEXTURE0 + i); // activate proper texture unit before binding
		// retrieve texture number (the N in diffuse_textureN)
		std::string number;
		std::string name = textures[i].type;
		if (name == "texture_diffuse")
			number = std::to_string(++diffuseNr);
		else if (name == "texture_specular")
			number = std::to_string(++specularNr);
		else if (name == "texture_normal")
			number = std::to_string(++normalNr);
		else if (name == "texture_height")
			number = std::to_string(++heightNr);

		// now set the sampler to the correct texture unit
		shader.setInt((name + number).c_str(), i);
		// and finally bind the texture
		glBindTexture(GL_TEXTURE_2D, textures[i].id);
	}
	
	// reactivate tex0 as this is the assumed state
	glActiveTexture(GL_TEXTURE0);
}

void Mesh::draw(Shader shader, bool shouldSendTextures) {
	if (shouldSendTextures)
		sendTexturesToShader(shader);

	// draw mesh
	glBindVertexArray(*VAO);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
}

void Mesh::setupMesh() {
	VAO = std::move(std::unique_ptr < GLuint, std::function<void(GLuint*)>>{ new GLuint(0), std::bind(&deleteVAO, std::placeholders::_1) });
	// create buffers/arrays
	GLuint VBO, EBO;
	glGenVertexArrays(1, VAO.get());
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(*VAO);
	// load data into vertex buffers
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

	// set the vertex attribute pointers
	// vertex Positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	// vertex normals
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
	// vertex texture coords
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
	// vertex tangent
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
	// vertex bitangent
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));

	glBindVertexArray(0);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
}