#include "stdafx.h"
#include "shader.hpp"

Shader::Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath) {
	// 1. retrieve the vertex/fragment source code from filePath
	std::string vertexCode;
	std::string fragmentCode;
	std::string geometryCode;
	std::ifstream vShaderFile;
	std::ifstream fShaderFile;
	std::ifstream gShaderFile;
	// ensure ifstream objects can throw exceptions:
	vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	gShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try {
		// open files, switching to the fallback shader directory if the vertex shader is not located at the default shader directory
		bool shadersFoundAtBaseDir = std::filesystem::exists(shaderDir + vertexPath);
		if (shadersFoundAtBaseDir) {
			vShaderFile.open(shaderDir + vertexPath);
			fShaderFile.open(shaderDir + fragmentPath);
		}
		else {
#ifdef WARNING
			WARNING(std::cout << "unable to locate shader at path '" << shaderDir + vertexPath << "'; searching fallback directory instead" << std::endl);
#endif
			vShaderFile.open(fallbackShaderDir + vertexPath);
			fShaderFile.open(fallbackShaderDir + fragmentPath);
		}
		std::stringstream vShaderStream, fShaderStream;
		// read file's buffer contents into streams
		vShaderStream << vShaderFile.rdbuf();
		fShaderStream << fShaderFile.rdbuf();
		// close file handlers
		vShaderFile.close();
		fShaderFile.close();
		// convert stream into string
		vertexCode = vShaderStream.str();
		fragmentCode = fShaderStream.str();
		// if geometry shader path is present, also load a geometry shader
		if (geometryPath != nullptr) {
			gShaderFile.open(shadersFoundAtBaseDir ? shaderDir + geometryPath : fallbackShaderDir + geometryPath);
			std::stringstream gShaderStream;
			gShaderStream << gShaderFile.rdbuf();
			gShaderFile.close();
			geometryCode = gShaderStream.str();
		}
	}
	catch (std::ifstream::failure e) {
		ERROR(std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl);
	}
	const char* vShaderCode = vertexCode.c_str();
	const char* fShaderCode = fragmentCode.c_str();
	// 2. compile shaders
	unsigned int vertex, fragment;
	// vertex shader
	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vShaderCode, NULL);
	glCompileShader(vertex);
	checkCompileErrors(vertex, "VERTEX");
	// fragment Shader
	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fShaderCode, NULL);
	glCompileShader(fragment);
	checkCompileErrors(fragment, "FRAGMENT");
	// if geometry shader is given, compile geometry shader
	unsigned int geometry;
	if (geometryPath != nullptr) {
		const char* gShaderCode = geometryCode.c_str();
		geometry = glCreateShader(GL_GEOMETRY_SHADER);
		glShaderSource(geometry, 1, &gShaderCode, NULL);
		glCompileShader(geometry);
		checkCompileErrors(geometry, "GEOMETRY");
	}
	// shader Program
	ID = glCreateProgram();
	glAttachShader(ID, vertex);
	glAttachShader(ID, fragment);
	if (geometryPath != nullptr)
		glAttachShader(ID, geometry);
	glLinkProgram(ID);
	checkCompileErrors(ID, "PROGRAM");
	// delete the shaders as they're linked into our program now and no longer necessery
	glDeleteShader(vertex);
	glDeleteShader(fragment);
	if (geometryPath != nullptr)
		glDeleteShader(geometry);
}

void Shader::use() {
	glUseProgram(ID);
}

void Shader::setBool(std::string name, bool value) {
	if (!uniformLocations.contains(name))
		uniformLocations[name] = glGetUniformLocation(ID, name.c_str());
	glUniform1i(uniformLocations[name], (int)value);
}

void Shader::setInt(std::string name, int value) {
	if (!uniformLocations.contains(name))
		uniformLocations[name] = glGetUniformLocation(ID, name.c_str());
	glUniform1i(uniformLocations[name], value);
}

void Shader::setFloat(std::string name, float value) {
	if (!uniformLocations.contains(name))
		uniformLocations[name] = glGetUniformLocation(ID, name.c_str());
	glUniform1f(uniformLocations[name], value);
}

void Shader::setVec2(std::string name, const glm::vec2& value) {
	if (!uniformLocations.contains(name))
		uniformLocations[name] = glGetUniformLocation(ID, name.c_str());
	glUniform2fv(uniformLocations[name], 1, &value[0]);
}
void Shader::setVec2(std::string name, float x, float y) {
	if (!uniformLocations.contains(name))
		uniformLocations[name] = glGetUniformLocation(ID, name.c_str());
	glUniform2f(uniformLocations[name], x, y);
}

void Shader::setVec3(std::string name, const glm::vec3& value) {
	if (!uniformLocations.contains(name))
		uniformLocations[name] = glGetUniformLocation(ID, name.c_str());
	glUniform3fv(uniformLocations[name], 1, &value[0]);
}

void Shader::setVec3(std::string name, float x, float y, float z) {
	if (!uniformLocations.contains(name))
		uniformLocations[name] = glGetUniformLocation(ID, name.c_str());
	glUniform3f(uniformLocations[name], x, y, z);
}

void Shader::setVec4(std::string name, const glm::vec4& value) {
	if (!uniformLocations.contains(name))
		uniformLocations[name] = glGetUniformLocation(ID, name.c_str());
	glUniform4fv(uniformLocations[name], 1, &value[0]);
}

void Shader::setVec4(std::string name, float x, float y, float z, float w) {
	if (!uniformLocations.contains(name))
		uniformLocations[name] = glGetUniformLocation(ID, name.c_str());
	glUniform4f(uniformLocations[name], x, y, z, w);
}

void Shader::setMat2(std::string name, const glm::mat2& mat) {
	if (!uniformLocations.contains(name))
		uniformLocations[name] = glGetUniformLocation(ID, name.c_str());
	glUniformMatrix2fv(uniformLocations[name], 1, GL_FALSE, &mat[0][0]);
}

void Shader::setMat3(std::string name, const glm::mat3& mat) {
	if (!uniformLocations.contains(name))
		uniformLocations[name] = glGetUniformLocation(ID, name.c_str());
	glUniformMatrix3fv(uniformLocations[name], 1, GL_FALSE, &mat[0][0]);
}

void Shader::setMat4(std::string name, const glm::mat4& mat) {
	if (!uniformLocations.contains(name))
		uniformLocations[name] = glGetUniformLocation(ID, name.c_str());
	glUniformMatrix4fv(uniformLocations[name], 1, GL_FALSE, &mat[0][0]);
}

void Shader::checkCompileErrors(GLuint shader, std::string type) {
	GLint success;
	GLchar infoLog[1024];
	if (type != "PROGRAM") {
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success) {
			glGetShaderInfoLog(shader, 1024, NULL, infoLog);
			ERROR(std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl);
		}
	}
	else {
		glGetProgramiv(shader, GL_LINK_STATUS, &success);
		if (!success) {
			glGetProgramInfoLog(shader, 1024, NULL, infoLog);
			ERROR(std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl);
		}
	}
}