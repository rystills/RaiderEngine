#include "stdafx.h"
#include "shader.hpp"
#include "mesh.hpp"
#include "GameObject.hpp"
#include "settings.hpp"
#include "model.hpp"

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

void Mesh::sendTexturesToShader(Shader shader, bool diffuseOnly) {
	if (diffuseOnly) {
		// bind the first diffuse texture
		for (int i = 0; i < textures.size(); ++i) {
			if (textures[i].type == MapType::texture_diffuse) {
				glActiveTexture(GL_TEXTURE0); // activate the proper texture unit before binding
				// set the sampler to the correct texture unit
				shader.setInt((Model::mapTypeNames[MapType::texture_diffuse] + '1').c_str(), 0);
				// bind the texture
				glBindTexture(GL_TEXTURE_2D, enableTextureMaps[MapType::texture_diffuse] ? textures[i].id : Model::mapDefaults[MapType::texture_diffuse]->id);
				break;
			}
		}
		return;
	}
	// bind appropriate textures
	unsigned int texMapNums[Model::numMapTypes] = { 0 };
	for (unsigned int i = 0; i < textures.size(); ++i) {
		glActiveTexture(GL_TEXTURE0 + i); // activate the proper texture unit before binding
		MapType type = textures[i].type;

		// set the sampler to the correct texture unit
		shader.setInt((Model::mapTypeNames[type] + std::to_string(++texMapNums[type])).c_str(), i);
		// bind the texture
		glBindTexture(GL_TEXTURE_2D, enableTextureMaps[type] ? textures[i].id : Model::mapDefaults[type]->id);
	}
	
	// reactivate tex0 as this is the assumed state
	glActiveTexture(GL_TEXTURE0);
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
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
	// vertex tangent
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));

	// instance model matrix
	glBindBuffer(GL_ARRAY_BUFFER, GameObject::instancedModelVBO);
	GLsizei vec4Size = sizeof(glm::vec4);
	glEnableVertexAttribArray(5);
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)0);
	glEnableVertexAttribArray(6);
	glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(vec4Size));
	glEnableVertexAttribArray(7);
	glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(2 * vec4Size));
	glEnableVertexAttribArray(8);
	glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(3 * vec4Size));

	glVertexAttribDivisor(5, 1);
	glVertexAttribDivisor(6, 1);
	glVertexAttribDivisor(7, 1);
	glVertexAttribDivisor(8, 1);

	glBindVertexArray(0);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
}