#include "stdafx.h"
#pragma once
#include "model.hpp"
#include "GameObject.hpp"
#include "input.hpp"
#include "settings.hpp"
// this file is responsible for graphics, including debug and text rendering
// settings
float anisoFilterAmount = 0.0f;
bool useVsync = true;
bool fullScreen = false;
#define numFontCharacters 128  // we only care about the first 128 characters stored in a given font file, at least for now

unsigned int VBO, VAO;
unsigned int textVBO, textVAO;

struct GBuffer {
	unsigned int buffer, position, normal, albedoSpec;
} gBuffer;

struct Character {
	GLuint     TextureID;  // ID handle of the glyph texture
	glm::ivec2 Size;       // Size of glyph
	glm::ivec2 Bearing;    // Offset from baseline to left/top of glyph
	GLuint     Advance;    // Offset to advance to next glyph
};
std::unordered_map<std::string, std::unordered_map<int, Character[numFontCharacters]>> fonts;

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

// renderQuad() renders a 1x1 XY quad in NDC
// -----------------------------------------
unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad() {
	if (quadVAO == 0) {
		float quadVertices[] = {
			// positions        // texture Coords
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		// setup plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

// renderCube() renders a 1x1 3D cube in NDC.
// -------------------------------------------------
unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;
void renderCube() {
	// initialize (if necessary)
	if (cubeVAO == 0) {
		float vertices[] = {
			// back face
			-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
			1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
			1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
			-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			-1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
			// front face
			-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
			1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
			1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
			1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
			-1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
			-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
			// left face
			-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
			-1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
			-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
			-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
			-1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
			-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
			// right face
			1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
			1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
			1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
			1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
			1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
			1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
			// bottom face
			-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
			1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
			1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
			1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
			-1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
			-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
			// top face
			-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
			1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
			1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
			1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
			-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
			-1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
		};
		glGenVertexArrays(1, &cubeVAO);
		glGenBuffers(1, &cubeVBO);
		// fill buffer
		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		// link vertex attributes
		glBindVertexArray(cubeVAO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
	// render Cube
	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}

// enable anisotropic filtering for all textures, if supported on the user's graphics card
void applyAnisotropicFiltering() {
	if (glfwExtensionSupported("GL_EXT_texture_filter_anisotropic"))
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &anisoFilterAmount);
	if (anisoFilterAmount > 0) {
		for (std::pair<const std::string, Texture> kv : Model::texturesLoaded) {
			glBindTexture(GL_TEXTURE_2D, kv.second.id);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisoFilterAmount);
		}
		glBindTexture(GL_TEXTURE_2D, 0);
	};
}

/*
initialize the default vertex buffer and attribute objects (VBO and VAO, respectively)
*/
void initBuffers() {
	glGenBuffers(1, &VBO);
	glGenVertexArrays(1, &VAO);
}


/*
render the specified text with the specified (font,size) pair (if loaded) at the specified position, scale, and color
@param fontName: the name of the font
@param fontSize: the size of the font
@param s: the font shader
@param text: the text string to render
@param x: the x coordinate (in screen pixels) at which to render the text
@param y: the y coordinate (in screen pixels) at which to render the text
@param scale: the scale at which to render the text
@param color: the color to use when rendering the text
@param centered: whether or not to center the rendered text
*/
void renderText(std::string fontName, int fontSize, Shader &s, std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color, bool centered = false) {
	if (!fonts[fontName].count(fontSize)) {
		ERROR(std::cout << "Error: font '" << fontName << "' at size '" << fontSize << "' not found in fonts map; please load this (font,size) pair and try again" << std::endl);
		return;
	}
	if (text.length() == 0)
		return;
	// Activate corresponding render state	
	s.use();
	glUniform3f(glGetUniformLocation(s.ID, "textColor"), color.x, color.y, color.z);
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(textVAO);

	std::string::const_iterator c;
	Character ch;
	// adjust text starting position if rendering centered text
	if (centered) {
		// TODO: using the size of a capital 'A' to get the effective height for now; a proper, cross-language solution should be implemented instead
		y -= fonts[fontName][fontSize]['A'].Size.y * scale / 2;
		// subtract half of the advance of each character, except for the last one (since we don't advance from the last character)
		for (c = text.begin(); c != text.end() - 1; ++c) {
			ch = fonts[fontName][fontSize][*c];
			x -= (ch.Advance >> 6) * scale / 2;
		}
		ch = fonts[fontName][fontSize][*(text.end() - 1)];
		// subtract the bearing and size of the last character instead of its advance, since we want its exact width
		// note: horizontal text centering may be slightly off; the logic seems correct, but the addition by 1 is a bit suspicious
		x -= (ch.Bearing.x + ch.Size.x) * scale / 2 + 1;
	}

	// Iterate through all characters
	for (c = text.begin(); c != text.end(); ++c) {
		ch = fonts[fontName][fontSize][*c];

		GLfloat xpos = x + ch.Bearing.x * scale;
		GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

		GLfloat w = ch.Size.x * scale;
		GLfloat h = ch.Size.y * scale;
		// Update VBO for each character
		GLfloat vertices[6][4] = {
			{ xpos,     ypos + h,   0.0, 0.0 },
			{ xpos,     ypos,       0.0, 1.0 },
			{ xpos + w, ypos,       1.0, 1.0 },

			{ xpos,     ypos + h,   0.0, 0.0 },
			{ xpos + w, ypos,       1.0, 1.0 },
			{ xpos + w, ypos + h,   1.0, 0.0 }
		};
		// Render glyph texture over quad
		glBindTexture(GL_TEXTURE_2D, ch.TextureID);
		// Update content of VBO memory
		glBindBuffer(GL_ARRAY_BUFFER, textVBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		// Render quad
		glDrawArrays(GL_TRIANGLES, 0, 6);
		// Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64)
	}
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

/*
initialize freetype, creating a VBO and VAO specifically for text rendering
*/
void initFreetype() {
	// Configure VAO/VBO for texture quads
	glGenVertexArrays(1, &textVAO);
	glGenBuffers(1, &textVBO);
	glBindVertexArray(textVAO);
	glBindBuffer(GL_ARRAY_BUFFER, textVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

/*
load the specified font at the desired size using freetype, adding the (fontName,size) pair to the fonts map
@param fontName: the name of the font to load
@param fontSize: the size at which to load the font
*/
void freetypeLoadFont(std::string fontName, int fontSize) {
	// FreeType
	FT_Library ft;
	// All functions return a value different than 0 whenever an error occurred
	if (FT_Init_FreeType(&ft))
		std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;

	// Load font as face
	FT_Face face;
	if (FT_New_Face(ft, ("fonts/" + fontName + ".ttf").c_str(), 0, &face))
		std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;

	// Set size to load glyphs as
	FT_Set_Pixel_Sizes(face, 0, fontSize);

	// Disable byte-alignment restriction
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// Load first 128 characters of ASCII set
	for (GLubyte c = 0; c < numFontCharacters; ++c) {
		// Load character glyph 
		if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
			std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
			continue;
		}
		// Generate texture
		GLuint texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RED,
			face->glyph->bitmap.width,
			face->glyph->bitmap.rows,
			0,
			GL_RED,
			GL_UNSIGNED_BYTE,
			face->glyph->bitmap.buffer
		);
		// Set texture options
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// Now store character for later use 
		fonts[fontName][fontSize][c] = {
			texture,
			glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
			glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
			static_cast<GLuint>(face->glyph->advance.x)
		};
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	// reset texture alignment
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	// Destroy FreeType once we're finished
	FT_Done_Face(face);
	FT_Done_FreeType(ft);
}

/*
draw a single point (useless unless the debug render shader is active)
@param pos: the point's position
@param color: the point's rgb color
*/
void debugDrawPoint(glm::vec3 pos, glm::vec3 color) {
	GLfloat points[6];
	points[0] = pos.x;
	points[1] = pos.y;
	points[2] = pos.z;
	points[3] = color.r;
	points[4] = color.g;
	points[5] = color.b;

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(points), &points, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glBindVertexArray(0);

	glBindVertexArray(VAO);
	glDrawArrays(GL_POINTS, 0, 2);
	glBindVertexArray(0);
}

/*
draw a single line (useless unless the debug render shader is active)
@param from: the line's start position
@param to: the line's end position
@param color: the line's rgb color
*/
void debugDrawLine(const glm::vec3& from, const glm::vec3 &to, const glm::vec3& color) {
	// Vertex data
	GLfloat points[12];

	points[0] = from.x;
	points[1] = from.y;
	points[2] = from.z;
	points[3] = color.x;
	points[4] = color.y;
	points[5] = color.z;

	points[6] = to.x;
	points[7] = to.y;
	points[8] = to.z;
	points[9] = color.x;
	points[10] = color.y;
	points[11] = color.z;

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(points), &points, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glBindVertexArray(0);

	glBindVertexArray(VAO);
	glDrawArrays(GL_LINES, 0, 2);
	glBindVertexArray(0);
}

glm::vec3 stateColors[3] = { glm::vec3(255,0,0), glm::vec3(0,0,255), glm::vec3(255,255,255) };
/*
callback for debugDrawNewton - provides us with a single body's transformed collider info
@param userData: newton's userData field; currently we use this as an enum where 2 = static, 1 = dynamic and asleep, 0 = dynamic and awake
@param vertexCount: the total number of vertices
@param faceVertec: array of face vertices
@param faceId:
*/
void debugDrawNewtonCallback(void* const userData, int vertexCount, const dFloat* const faceVertec, int faceId) {
	int state = (int)userData;
	int index = vertexCount - 1;
	dVector p0(faceVertec[index * 3 + 0], faceVertec[index * 3 + 1], faceVertec[index * 3 + 2]);
	for (int i = 0; i < vertexCount; i++) {
		dVector p1(faceVertec[i * 3 + 0], faceVertec[i * 3 + 1], faceVertec[i * 3 + 2]);
		debugDrawLine(glm::vec3(GLfloat(p0.m_x), GLfloat(p0.m_y), GLfloat(p0.m_z)), glm::vec3(GLfloat(p1.m_x), GLfloat(p1.m_y), GLfloat(p1.m_z)), stateColors[state]);
		p0 = p1;
	}
}

/*
draw all newton colliders as wireframes
*/
void debugDrawNewton() {
	for (int i = 0; i < gameObjects.size(); ++i) {
		dMatrix tm;
		NewtonBodyGetMatrix(gameObjects[i]->body, &tm[0][0]);
		NewtonCollisionForEachPolygonDo(NewtonBodyGetCollision(gameObjects[i]->body), &tm[0][0], debugDrawNewtonCallback, gameObjects[i]->model->isStaticMesh ? (void*)2 : (void*)NewtonBodyGetSleepState(gameObjects[i]->body));
	}
}

/*
initialize the contents of the g buffer used for deferred rendering
*/
void initGBuffer() {
	glGenFramebuffers(1, &gBuffer.buffer);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer.buffer);
	// position color buffer
	glGenTextures(1, &gBuffer.position);
	glBindTexture(GL_TEXTURE_2D, gBuffer.position);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gBuffer.position, 0);
	// normal color buffer
	glGenTextures(1, &gBuffer.normal);
	glBindTexture(GL_TEXTURE_2D, gBuffer.normal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gBuffer.normal, 0);
	// color + specular color buffer
	glGenTextures(1, &gBuffer.albedoSpec);
	glBindTexture(GL_TEXTURE_2D, gBuffer.albedoSpec);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gBuffer.albedoSpec, 0);
	// tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
	unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, attachments);
	// create and attach depth buffer (renderbuffer)
	unsigned int rboDepth;
	glGenRenderbuffers(1, &rboDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
	// finally check if framebuffer is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		ERROR(std::cout << "Framebuffer not complete!" << std::endl);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

/*
initialize our game window, creating the window itself and setting input callbacks
*/
GLFWwindow* initWindow() {
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// setup windows console colors here since the original console color doesn't appear to be accessible prior to main
#ifdef _WIN32
	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	GetConsoleScreenBufferInfo(hConsole, &cbInfo);
	originalColor = cbInfo.wAttributes;
#endif

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "CPPGameEngine", fullScreen ? glfwGetPrimaryMonitor() : NULL, NULL);
	if (window == NULL) {
		ERROR(std::cout << "Failed to create GLFW window" << std::endl);
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		ERROR(std::cout << "Failed to initialize GLAD" << std::endl);
		exit(EXIT_FAILURE);
	}

	glfwSwapInterval(useVsync);
	return window;
}