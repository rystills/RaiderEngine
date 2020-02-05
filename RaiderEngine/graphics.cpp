#include "stdafx.h"
#include "settings.hpp"
#include "model.hpp"
#include "GameObject.hpp"
#include "GameObject2D.hpp"
#include "Light.hpp"
#include "TextObject.hpp"
#include "input.hpp"
#include "graphics.hpp"
#include "terminalColors.hpp"
#include "physics.hpp"
#include "ParticleEmitter2D.hpp"

void frameBufferSizeCallback(GLFWwindow* window, int width, int height) {
	// make sure the viewport matches the new window dimensions; note that width and height will be significantly larger than specified on retina displays.
	// ignore invalid width/height values generated when minimizing the window
	if (width == 0 || height == 0 || (width == SCR_WIDTH && height == SCR_HEIGHT))
		return;
	SCR_WIDTH = width;
	SCR_HEIGHT = height;
	glfwSetWindowSize(window, SCR_WIDTH, SCR_HEIGHT);
	calcOrthoProjection();
	glDeleteFramebuffers(1, &gBuffer.buffer);
	initGBuffer();
	glViewport(0, 0, width, height);
}

void glfwErrorCallback(int errorno, const char* errmsg) {
	ERROR(std::cout << "GLFW Error #" << errorno << ": " << errmsg << std::endl);
}

void initQuad() {
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

void renderQuad() {
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

void initCube() {
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

void renderCube() {
	// render Cube
	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}

void applyAnisotropicFiltering() {
	if (glfwExtensionSupported("GL_EXT_texture_filter_anisotropic"))
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &anisoFilterAmount);
	if (anisoFilterAmount > 0) {
		for (std::pair<const std::string, Texture> kv : Model::texturesLoaded) {
			glBindTexture(GL_TEXTURE_2D, kv.second.id);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisoFilterAmount);
		}
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

void initBuffers() {
	glGenBuffers(1, &primitiveVBO);
	glGenVertexArrays(1, &primitiveVAO);
}

void renderText(std::string fontName, int fontSize, Shader& s, std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color, bool centered) {
	// make sure we're using a valid font/size, and rendering a non-empty string
	if (!fonts[fontName].count(fontSize)) {
		ERROR(std::cout << "Error: font '" << fontName << "' at size '" << fontSize << "' not found in fonts map; please load this (font,size) pair and try again" << std::endl);
		return;
	}
	if (text.length() == 0)
		return;

	// activate and prepare the shader
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(TextObject::VAO);
	glBindBuffer(GL_ARRAY_BUFFER, TextObject::VBO);
	// only rebind the font atlas if we switched fonts, font sizes, or shaders (texture binding is not tied to the active shader, so other shaders likely wrote over our texture)
	if (s.use() || lastFontUsed.first != fontName || lastFontUsed.second != fontSize) {
		lastFontUsed = { fontName, fontSize };
		glBindTexture(GL_TEXTURE_2D, fonts[fontName][fontSize].first);
	}
	if (lastFontColor != color) {
		lastFontColor = color;
		s.setVec3("textColor", color.x, color.y, color.z);
	}

	if (text.length() > TextObject::numGlpyhsBuffered) {
		TextObject::numGlpyhsBuffered = text.length();
		glBufferData(GL_ARRAY_BUFFER, TextObject::numGlpyhsBuffered * 24 * sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW);
	}

	Character ch;
	// adjust text starting position if rendering centered text
	if (centered) {
		// TODO: using the size of a capital 'A' to get the effective height for now; a proper, cross-language solution should be implemented instead
		y -= fonts[fontName][fontSize].second['A'].y_size * scale / 2;
		// subtract half of the advance of each character, except for the last one (since we don't advance from the last character)
		for (std::string::const_iterator c = text.begin(); c != text.end() - 1; ++c) {
			ch = fonts[fontName][fontSize].second[*c];
			x -= (ch.advance) * scale / 2;
		}
		ch = fonts[fontName][fontSize].second[*(text.end() - 1)];
		// subtract the bearing and size of the last character instead of its advance, since we want its exact width
		// note: horizontal text centering may be slightly off; the logic seems correct, but the addition by 1 is a bit suspicious
		x -= (ch.x_off + ch.x_size) * scale / 2 + 1;
	}

	std::vector<GLfloat> verts;
	verts.reserve(24 * text.length());	
	// Iterate through all characters
	for (int i = 0; i < text.length(); ++i) {
		ch = fonts[fontName][fontSize].second[text[i]];

		GLfloat xpos = x + ch.x_off * scale;
		GLfloat ypos = y - (ch.y_size - ch.y_off) * scale;
		GLfloat w = ch.x_size * scale;
		GLfloat h = ch.y_size * scale;

		// TODO: try instanced rendering + tristrips (see GameObject2D / ParticleEmitter2D) for a minor performance boost
		// add character position and uv data to the vertex vector
		verts[24 * i]    = xpos;
		verts[24 * i+1]  = ypos + h;
		verts[24 * i+2]  = ch.x0;
		verts[24 * i+3]  = ch.y0;

		verts[24 * i+4]  = xpos;
		verts[24 * i+5]  = ypos;
		verts[24 * i+6]  = ch.x0;
		verts[24 * i+7]  = ch.y1;
		
		verts[24 * i+8]  = xpos + w;
		verts[24 * i+9]  = ypos;
		verts[24 * i+10] = ch.x1;
		verts[24 * i+11] = ch.y1;
		
		verts[24 * i+12]  = xpos;
		verts[24 * i+13]  = ypos + h;
		verts[24 * i+14] = ch.x0;
		verts[24 * i+15] = ch.y0;
		
		verts[24 * i+16]  = xpos + w;
		verts[24 * i+17]  = ypos;
		verts[24 * i+18] = ch.x1;
		verts[24 * i+19] = ch.y1;
		
		verts[24 * i+20] = xpos + w;
		verts[24 * i+21] = ypos + h;
		verts[24 * i+22] = ch.x1;
		verts[24 * i+23] = ch.y0;

		// Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		x += (ch.advance) * scale;
	}

	// render the full set of glyphs as a single triangle array
	glBufferSubData(GL_ARRAY_BUFFER, 0, 24 * sizeof(GLfloat) * text.length(), &verts[0]);
	glDrawArrays(GL_TRIANGLES, 0, text.length()*6);
	
	glBindVertexArray(0);
}

void freetypeLoadFont(std::string fontName, int fontSize) {
	FT_Library ft;
	FT_Face    face;

	FT_Init_FreeType(&ft);
	if (FT_New_Face(ft, (fontDir + fontName + ".ttf").c_str(), 0, &face))
		ERROR(std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl)
	FT_Set_Pixel_Sizes(face, 0, fontSize);

	// calculate texture width
	int max_dim = (1 + (face->size->metrics.height >> 6))* ceilf(sqrtf(numFontCharacters));
	int tex_width = 1;
	while (tex_width < max_dim)
		tex_width <<= 1;
	// tex_height is initially set to match tex_width; once we're done loading in the glyphs, we'll be able to create the final texture with the exact height
	int tex_height = tex_width;

	// render glyphs to atlas
	char* pixels = (char*)calloc(tex_width * tex_height, 1);
	int pen_x = 0, pen_y = 0;

	for (int i = 0; i < numFontCharacters; ++i) {
		FT_Load_Char(face, i, FT_LOAD_RENDER);
		FT_Bitmap* bmp = &face->glyph->bitmap;

		if (pen_x + bmp->width >= tex_width) {
			pen_x = 0;
			pen_y += ((face->size->metrics.height >> 6) + 1);
		}

		for (int row = 0; row < bmp->rows; ++row) {
			for (int col = 0; col < bmp->width; ++col) {
				int x = pen_x + col;
				int y = pen_y + row;
				pixels[y * tex_width + x] = bmp->buffer[row * bmp->pitch + col];
			}
		}

		// populate Character struct with info needed for rendering
		fonts[fontName][fontSize].second[i].x0 = pen_x;
		fonts[fontName][fontSize].second[i].y0 = pen_y;
		fonts[fontName][fontSize].second[i].x1 = pen_x + bmp->width;
		fonts[fontName][fontSize].second[i].y1 = pen_y + bmp->rows;

		fonts[fontName][fontSize].second[i].x_size = bmp->width;
		fonts[fontName][fontSize].second[i].y_size = bmp->rows;

		fonts[fontName][fontSize].second[i].x_off = face->glyph->bitmap_left;
		fonts[fontName][fontSize].second[i].y_off = face->glyph->bitmap_top;
		fonts[fontName][fontSize].second[i].advance = face->glyph->advance.x >> 6;

		pen_x += bmp->width + 1;
	}

	// Generate texture
	glGenTextures(1, &fonts[fontName][fontSize].first);
	glBindTexture(GL_TEXTURE_2D, fonts[fontName][fontSize].first);
	tex_height = pen_y + ((face->size->metrics.height >> 6) + 1);

	// bake x0,x1,y0,y1 in uv space
	for (int i = 0; i < numFontCharacters; ++i) {
		fonts[fontName][fontSize].second[i].x0 /= tex_width;
		fonts[fontName][fontSize].second[i].x1 /= tex_width;
		fonts[fontName][fontSize].second[i].y0 /= tex_height;
		fonts[fontName][fontSize].second[i].y1 /= tex_height;
	}

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, tex_width, tex_height, 0, GL_RED, GL_UNSIGNED_BYTE, pixels);
	// Set texture options
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	FT_Done_FreeType(ft);
	free(pixels);
}

void queueDrawPoint(glm::vec3 pos, glm::vec3 color) {
	GLfloat points[6];
	points[0] = pos.x;
	points[1] = pos.y;
	points[2] = pos.z;
	points[3] = color.r;
	points[4] = color.g;
	points[5] = color.b;

	pointsQueue.insert(std::end(pointsQueue), std::begin(points), std::end(points));
}

void drawPoints() {
	// TODO: clean up this method (switch to glBufferSubData, remove needless state changes)
	// todo: offset screen coords by 0.5f to draw points from pixel centers rather than corners?
	glBindVertexArray(primitiveVAO);
	glBindBuffer(GL_ARRAY_BUFFER, primitiveVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * pointsQueue.size(), &pointsQueue[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glBindVertexArray(0);

	glBindVertexArray(primitiveVAO);
	glDrawArrays(GL_POINTS, 0, pointsQueue.size() / 6);
	glBindVertexArray(0);
	pointsQueue.clear();
}

void queueDrawLine(const glm::vec3& from, const glm::vec3& to, const glm::vec3& color) {
	// todo: offset screen coords by 0.5f to draw lines from pixel centers rather than corners?
	GLfloat points[18];

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

	// terminate each line with NANs so that we can draw the entire array of disjointed lines as a single linestrip
	// TODO: consider replacing this with a primitive restart to save a bit of memory and render performance
	points[12] = NAN;
	points[13] = NAN;
	points[14] = NAN;
	points[15] = NAN;
	points[16] = NAN;
	points[17] = NAN;

	linesQueue.insert(std::end(linesQueue), std::begin(points), std::end(points));
}

void drawLines() {
	// TODO: clean up this method (switch to glBufferSubData, remove needless state changes)
	glBindVertexArray(primitiveVAO);
	glBindBuffer(GL_ARRAY_BUFFER, primitiveVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * linesQueue.size(), &linesQueue[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glBindVertexArray(0);

	glBindVertexArray(primitiveVAO);
	glDrawArrays(GL_LINE_STRIP, 0, linesQueue.size() / 6);
	glBindVertexArray(0);
	linesQueue.clear();
}

void calcOrthoProjection() {
	glmOrthoProjection = glm::ortho(0.0f, static_cast<GLfloat>(SCR_WIDTH), static_cast<GLfloat>(SCR_HEIGHT), 0.0f, -1.0f, 1.0f);
	glmOrthoTextProjection = glm::ortho(0.0f, static_cast<GLfloat>(SCR_WIDTH), 0.0f, static_cast<GLfloat>(SCR_HEIGHT));

	// apply to all shaders
	shaders["textShader"]->use();
	shaders["textShader"]->setMat4("projection", glmOrthoTextProjection);
	shaders["lineShader2D"]->use();
	shaders["lineShader2D"]->setMat4("projection", glmOrthoProjection);
	shaders["2DShader"]->use();
	shaders["2DShader"]->setMat4("projection", glmOrthoProjection);
	shaders["Particle2DShader"]->use();
	shaders["Particle2DShader"]->setMat4("projection", glmOrthoProjection);
}

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

void initDepthMaps() {
	for (int i = 0; i < NR_LIGHTS; ++i) {
		glGenFramebuffers(1, &depthMapFBO[i]);
		// create depth cubemap texture
		glGenTextures(1, &depthCubemap[i]);
		glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap[i]);
		for (unsigned int i = 0; i < 6; ++i)
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		// attach depth texture as FBO's depth buffer
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO[i]);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap[i], 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}

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
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "RaiderEngine", fullScreen ? glfwGetPrimaryMonitor() : NULL, NULL);
	if (window == NULL) {
		ERROR(std::cout << "Failed to create GLFW window" << std::endl);
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	// callback events
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, frameBufferSizeCallback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetKeyCallback(window, key_callback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		ERROR(std::cout << "Failed to initialize GLAD" << std::endl);
		exit(EXIT_FAILURE);
	}

	glfwSwapInterval(useVsync);

	// required capabilities
	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
	glEnable(GL_PROGRAM_POINT_SIZE);

	return window;
}

void loadShaders() {
	// load shaders
	shaders["shaderGeometryPass"] = std::make_unique<Shader>("g_buffer.vs", "g_buffer.fs");
	shaders["shaderLightingPass"] = std::make_unique<Shader>("deferred_shading.vs", "deferred_shading.fs");
	shaders["shaderLightBox"] = std::make_unique<Shader>("deferred_light_box.vs", "deferred_light_box.fs");
	shaders["lineShader"] = std::make_unique<Shader>("lineShader.vs", "lineShader.fs");
	shaders["lineShader2D"] = std::make_unique<Shader>("lineShader2D.vs", "lineShader2D.fs");
	shaders["textShader"] = std::make_unique<Shader>("textShader.vs", "textShader.fs");
	shaders["2DShader"] = std::make_unique<Shader>("2DShader.vs", "2DShader.fs");
	shaders["Particle2DShader"] = std::make_unique<Shader>("Particle2DShader.vs", "Particle2DShader.fs");
	shaders["pointShadowsDepth"] = std::make_unique<Shader>("point_shadows_depth.vs", "point_shadows_depth.fs", "point_shadows_depth.gs");

	// configure shaders
	shaders["shaderLightingPass"]->use();
	shaders["shaderLightingPass"]->setInt("gPosition", 0);
	shaders["shaderLightingPass"]->setInt("gNormal", 1);
	shaders["shaderLightingPass"]->setInt("gAlbedoSpec", 2);
	shaders["shaderLightingPass"]->setInt("depthMap0", 3);
	shaders["shaderLightingPass"]->setInt("depthMap1", 4);
	shaders["shaderLightingPass"]->setInt("depthMap2", 5);
	shaders["shaderLightingPass"]->setInt("depthMap3", 6);

	shaders["2DShader"]->use();
	shaders["2DShader"]->setInt("image", 0);
}

void drawGameObjects(std::string shaderName, bool shouldSendTextures, bool ignoreNonShadowCasters) {
	for (auto&& kv : gameObjects) {
		if (kv.second.size() == 0)
			continue;
		if (ignoreNonShadowCasters && !kv.second[0]->castShadows)
			continue;
		// toggle face culling when drawing a double-sided object
		if (kv.second[0]->drawTwoSided != twoSidedDrawing) {
			if (kv.second[0]->drawTwoSided)
				glDisable(GL_CULL_FACE);
			else
				glEnable(GL_CULL_FACE);
			twoSidedDrawing = !twoSidedDrawing;
		}
		// no instancing can be done if only a single GameObject uses this model
		if (kv.second.size() == 1) {
			shaders[shaderName]->setMat4("model", glm::scale(glm::translate(glm::mat4(1.0f), kv.second[0]->position) * kv.second[0]->rotation, kv.second[0]->scale));
			kv.second[0]->model->draw(*shaders[shaderName], shouldSendTextures);
		}
		else {
			// iterate through all submeshes
			for (int i = 0; i < kv.second[0]->model->meshes.size(); ++i) {
				// transfer the textures for the current submesh once initially
				if (shouldSendTextures)
					kv.second[0]->model->meshes[i].sendTexturesToShader(*shaders[shaderName]);
				// now render the current submesh for all GameObjects
				// TODO: further optimize this with instanced rendering via glDrawElementsInstanced (will require some alterations to the shader)
				for (int r = 0; r < kv.second.size(); ++r) {
					shaders[shaderName]->setMat4("model", glm::scale(glm::translate(glm::mat4(1.0f), kv.second[r]->position) * kv.second[r]->rotation, kv.second[r]->scale));
					kv.second[r]->model->meshes[i].draw(*shaders[shaderName], false);
				}
			}
		}
	}
}

void renderDepthMap() {
	// create depth cubemap transformation matrices
	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	shaders["pointShadowsDepth"]->use();
	shaders["pointShadowsDepth"]->setFloat("far_plane", mainCam->far_plane);
	glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT, mainCam->near_plane, mainCam->far_plane);
	for (int k = 0; k < lights.size(); ++k) {
		if (lights[k]->on) {
			glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO[k]);
			glClear(GL_DEPTH_BUFFER_BIT);
			glm::vec3 lightPos = lights[k]->position;
			std::vector<glm::mat4> shadowTransforms;
			shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
			shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
			shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
			shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
			shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
			shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));

			// render scene to depth cubemap
			for (unsigned int i = 0; i < 6; ++i)
				shaders["pointShadowsDepth"]->setMat4("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
			shaders["pointShadowsDepth"]->setVec3("lightPos", lightPos);
			drawGameObjects("pointShadowsDepth", false, true);
		}
	}
}

void renderGeometryPass() {
	glDisable(GL_BLEND);
	// geometry pass: render scene's geometry/color data into gbuffer
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer.buffer);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	shaders["shaderGeometryPass"]->use();
	shaders["shaderGeometryPass"]->setMat4("projection", mainCam->projection);
	shaders["shaderGeometryPass"]->setMat4("view", mainCam->view);
	shaders["shaderGeometryPass"]->setVec3("viewPos", mainCam->Position);
	drawGameObjects("shaderGeometryPass");

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void renderLightingPass() {
	// lighting pass: calculate lighting by iterating over a screen filled quad pixel-by-pixel using the gbuffer's content.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	shaders["shaderLightingPass"]->use();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gBuffer.position);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gBuffer.normal);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, gBuffer.albedoSpec);
	// send light relevant uniforms
	shaders["shaderLightingPass"]->setInt("NR_LIGHTS", lights.size());
	for (unsigned int i = 0; i < lights.size(); ++i) {
		if (lights[i]->on) {
			shaders["shaderLightingPass"]->setFloat("lights[" + std::to_string(i) + "].On", true);
			shaders["shaderLightingPass"]->setVec3("lights[" + std::to_string(i) + "].Position", lights[i]->position);
			shaders["shaderLightingPass"]->setVec3("lights[" + std::to_string(i) + "].Color", lights[i]->color);
			shaders["shaderLightingPass"]->setFloat("lights[" + std::to_string(i) + "].Linear", lights[i]->linear);
			shaders["shaderLightingPass"]->setFloat("lights[" + std::to_string(i) + "].Quadratic", lights[i]->quadratic);
			shaders["shaderLightingPass"]->setFloat("lights[" + std::to_string(i) + "].Radius", lights[i]->radius);
		}
		else
			shaders["shaderLightingPass"]->setFloat("lights[" + std::to_string(i) + "].On", false);
	}
	shaders["shaderLightingPass"]->setVec3("viewPos", mainCam->Position);
	// shadow and lighting uniforms
	shaders["shaderLightingPass"]->setFloat("far_plane", mainCam->far_plane);
	shaders["shaderLightingPass"]->setFloat("ambientStrength", ambientStrength);
	shaders["shaderLightingPass"]->setVec4("clearColor", clearColor);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap[0]);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap[1]);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap[2]);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap[3]);


	// finally render quad
	renderQuad();

	// copy content of geometry's depth buffer to default framebuffer's depth buffer
	glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer.buffer);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // TODO: internal format of FBO and default framebuffer must match (implementation defined?)
	glBlitFramebuffer(0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0, SCR_WIDTH, SCR_HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void debugDrawLightCubes() {
	// render lights on top of scene
	shaders["shaderLightBox"]->use();
	shaders["shaderLightBox"]->setMat4("projection", mainCam->projection);
	shaders["shaderLightBox"]->setMat4("view", mainCam->view);
	for (unsigned int i = 0; i < lights.size(); i++) {
		shaders["shaderLightBox"]->setMat4("model", glm::scale(glm::translate(glm::mat4(1.0f), lights[i]->position), glm::vec3(.1f)));
		shaders["shaderLightBox"]->setVec3("lightColor", lights[i]->on ? lights[i]->color : lights[i]->offColor);
		renderCube();
	}
}

void renderLines() {
	// render UI
	glDisable(GL_DEPTH_TEST);
	shaders["lineShader"]->use();
	shaders["lineShader"]->setMat4("projection", mainCam->projection);
	shaders["lineShader"]->setMat4("view", mainCam->view);
	if (debugDraw)
		// draw 3d colliders
		debugDrawPhysics();
}

void renderLines2D() {
	glDisable(GL_DEPTH_TEST);
	shaders["lineShader2D"]->use();
	if (debugDraw) {
		// draw 2d colliders
		for (auto&& kv : gameObject2Ds)
			for (int i = 0; i < kv.second.size(); ++i)
				if (kv.second[i]->collider)
					kv.second[i]->collider->debugDraw(kv.second[i]->center, kv.second[i]->rotation);
	}
}

void render2D(bool clearScreen) {
	glEnable(GL_DEPTH_TEST);
	// clear the depth buffer so 2D objects don't compete with 3d objects for visibility
	glClear(clearScreen ? GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT : GL_DEPTH_BUFFER_BIT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// render GameObject2Ds
	shaders["2DShader"]->use();
	glBindVertexArray(GameObject2D::VAO);
	glActiveTexture(GL_TEXTURE0);
	for (auto&& kv : gameObject2Ds) {
		// don't render gameObjects using the default (empty) sprite
		if (kv.second[0]->sprite.id == Model::defaultDiffuseMap.id)
			continue;
		glBindTexture(GL_TEXTURE_2D, kv.second[0]->sprite.id);
		// copy GameObject2D model matrices before rendering them all in one go
		// TODO: consider caching model matrix array for static GameObject2Ds
		if (kv.second.size() > modelMatrices.capacity()) {
			modelMatrices.reserve(kv.second.size());
			colorVectors.reserve(kv.second.size());
			glBindBuffer(GL_ARRAY_BUFFER, GameObject2D::instancedModelVBO);
			glBufferData(GL_ARRAY_BUFFER, kv.second.size() * sizeof(glm::mat4), NULL, GL_DYNAMIC_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, GameObject2D::instancedColorVBO);
			glBufferData(GL_ARRAY_BUFFER, kv.second.size() * sizeof(glm::vec3), NULL, GL_DYNAMIC_DRAW);
		}
		for (int i = 0; i < kv.second.size(); ++i) {
			if (kv.second[i]->isDirty)
				kv.second[i]->recalculateModel();
			modelMatrices[i] = kv.second[i]->model;
			colorVectors[i] = kv.second[i]->color;
		}
		glBindBuffer(GL_ARRAY_BUFFER, GameObject2D::instancedModelVBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, kv.second.size() * sizeof(glm::mat4), &modelMatrices[0]);
		glBindBuffer(GL_ARRAY_BUFFER, GameObject2D::instancedColorVBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, kv.second.size() * sizeof(glm::vec3), &colorVectors[0]);
		glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, kv.second.size());
	}
	glBindVertexArray(0);

	// render Particle2Ds
	glDisable(GL_DEPTH_TEST);

	shaders["Particle2DShader"]->use();
	glBindVertexArray(ParticleEmitter2D::VAO);
	glActiveTexture(GL_TEXTURE0);
	glBindBuffer(GL_ARRAY_BUFFER, ParticleEmitter2D::VBO);
	glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);
	for (int i = 0; i < 2; glBlendFunc(GL_SRC_ALPHA, GL_ONE), ++i) {
		// render particles in two passes; once in black, and once normally. Results in additive blending between particles, and normal blending with everything else
		// TODO: there may be a way to optimize this. Consider rendering to a second buffer rather than doing two passes, or possible single-pass solution using premultiplied alpha
		for (auto&& pe : particleEmitter2Ds) {
			glBindTexture(GL_TEXTURE_2D, pe->sprite.id);
			shaders["Particle2DShader"]->setVec2("spriteDims", glm::vec2(pe->sprite.width, pe->sprite.height));
			if (pe->particles.size() > ParticleEmitter2D::numParticlesInVBO) {
				ParticleEmitter2D::numParticlesInVBO = pe->particles.size();
				glBufferData(GL_ARRAY_BUFFER, pe->particles.size() * 7 * sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW);
			}
			glBufferSubData(GL_ARRAY_BUFFER, 0, pe->particles.size() * 7 * sizeof(GLfloat), &pe->particles[0]);
			glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, pe->particles.size());
		}
	}
	glBindVertexArray(0);

	// render text
	// TODO: text rendering should be orderable too
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	for (int i = 0; i < textObjects.size(); ++i)
		textObjects[i]->draw(*shaders["textShader"]);
}

void initMainCamera() {
	mainCam = new Camera(glm::vec3(0));
}

GLFWwindow* initGraphics() {
	glfwSetErrorCallback(glfwErrorCallback);
	GLFWwindow* window = initWindow();
	initQuad();
	initCube();
	initGBuffer();
	initDepthMaps();
	initPhysics();
	initBuffers();
	initMainCamera();
	loadShaders();
	calcOrthoProjection();
	Model::createDefaultMaterialMaps();
	GameObject2D::initStaticVertexBuffer();
	ParticleEmitter2D::initVertexObjects();
	TextObject::initVertexObjects();
	return window;
}