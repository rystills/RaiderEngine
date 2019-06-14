#pragma once

#include <glad/glad.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "mesh.hpp"
#include "shader.hpp"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <memory>

class Light {
public:
	glm::vec3 position, color, offColor;
	float linear, quadratic;
	int constant = 1;
	float radius;
	float maxBrightness;
	bool on = true;
	Light(glm::vec3 position, float strength, glm::vec3 color) : position(position), color(color) {
		offColor = glm::vec3(color.r / 8, color.g / 8, color.b / 8);
		linear = 7/strength*.7f;
		quadratic = 7/strength*1.8;
		calculateMaxBrightness();
		calculateRadius();
	}
	
	virtual void update(float deltaTime) { }

private:
	/*
	calculate the lights maximum brightness using its rgb components
	*/
	void calculateMaxBrightness() {
		maxBrightness = std::fmaxf(std::fmaxf(color.r, color.g), color.b);
	}
	/*
	calcuate the light's radius given linear and quadratic constants and its max brightness
	*/
	void calculateRadius() {
		radius = (-linear + std::sqrt(linear * linear - 4 * quadratic * (1 - (256.0f / 5.0f) * maxBrightness))) / (2.0f * quadratic);
	}
};