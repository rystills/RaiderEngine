#pragma once
#include "stdafx.h"

class Shader {
public:
	static inline std::string shaderDir = "shaders/", fallbackShaderDir = "";
	static inline unsigned int activeShader = -1;
    unsigned int ID;

	Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr, const char* tessellationControlPath = nullptr, const char* tessellationEvaluationPath = nullptr);
    /*
	activate the shader
	@returns: whether or not we ended up switching the active shader
	*/
	bool use();
	/*
	util functions for setting shader vars
	*/
	void setBool(std::string name, bool value);

	void setInt(std::string name, int value);

	void setFloat(std::string name, float value);

	void setVec2(std::string name, const glm::vec2& value);

	void setVec2(std::string name, float x, float y);

	void setVec3(std::string name, const glm::vec3& value);

	void setVec3(std::string name, float x, float y, float z);

	void setVec4(std::string name, const glm::vec4& value);

	void setVec4(std::string name, float x, float y, float z, float w);

	void setMat2(std::string name, const glm::mat2& mat);

	void setMat3(std::string name, const glm::mat3& mat);

	void setMat4(std::string name, const glm::mat4& mat);

private:
	std::unordered_map <std::string, GLint> uniformLocations;
    /*
	utility function for checking shader compilation/linking errors
	*/
	void checkCompileErrors(GLuint shader, std::string type);
};