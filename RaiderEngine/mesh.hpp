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
};

/*
custom deleter for smart pointers containing opengl graphics buffers; deletes the buffer's contents before deleting the buffer itself
@param b: the graphics buffer to delete
*/
void deleteGraphicsBuffer(ALuint* b) {
	glDeleteBuffers(1, b);
	delete b;
}

class Mesh {
public:
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> textures;
	std::shared_ptr<unsigned int> VAO;

	/*
	Mesh constructor: creates a new mesh with the provided vertex, index, and texture data
	@param vertices: a vector of vertices that comprise the mesh
	@param indices: a vector of indices that comprise the mesh
	@param textures: a vector of textures that the mesh uses
	*/
    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures) {
        this->vertices = vertices;
        this->indices = indices;
        this->textures = textures;

        // now that we have all the required data, set the vertex buffers and its attribute pointers.
        setupMesh();
    }

    // render the mesh
    void draw(Shader shader) {
        // bind appropriate textures
        unsigned int diffuseNr = 0, specularNr = 0, normalNr = 0, heightNr = 0;
        for(unsigned int i = 0; i < textures.size(); i++) {
            glActiveTexture(GL_TEXTURE0 + i); // active proper texture unit before binding
            // retrieve texture number (the N in diffuse_textureN)
			std::string number;
			std::string name = textures[i].type;
            if(name == "texture_diffuse")
				number = std::to_string(++diffuseNr);
			else if(name == "texture_specular")
				number = std::to_string(++specularNr);
            else if(name == "texture_normal")
				number = std::to_string(++normalNr);
             else if(name == "texture_height")
			    number = std::to_string(++heightNr);

			// now set the sampler to the correct texture unit
            glUniform1i(glGetUniformLocation(shader.ID, (name + number).c_str()), i);
            // and finally bind the texture
            glBindTexture(GL_TEXTURE_2D, textures[i].id);
        }
        
        // draw mesh
        glBindVertexArray(*VAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // always good practice to set everything back to defaults once configured.
        glActiveTexture(GL_TEXTURE0);
    }

private:
	std::shared_ptr<unsigned int> VBO, EBO;

    /*
	initialize all the buffer objects/arrays
	*/
    void setupMesh() {
        // create buffers/arrays
		VAO = std::shared_ptr<unsigned int>(new unsigned int(0), deleteGraphicsBuffer);
		VBO = std::shared_ptr<unsigned int>(new unsigned int(0), deleteGraphicsBuffer);
		EBO = std::shared_ptr<unsigned int>(new unsigned int(0), deleteGraphicsBuffer);
        glGenVertexArrays(1, &*VAO);
        glGenBuffers(1, &*VBO);
        glGenBuffers(1, &*EBO);

        glBindVertexArray(*VAO);
        // load data into vertex buffers
        glBindBuffer(GL_ARRAY_BUFFER, *VBO);
        // A great thing about structs is that their memory layout is sequential for all its items.
        // The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
        // again translates to 3/2 floats which translates to a byte array.
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);  

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *EBO);
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
    }
};