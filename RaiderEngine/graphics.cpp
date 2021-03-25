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
#include "Tilemap.hpp"
#include "timing.hpp"
#include "PlayerSpawn.hpp"

void frameBufferSizeCallback(GLFWwindow* window, int width, int height) {
	// make sure the viewport matches the new window dimensions; note that width and height will be significantly larger than specified on retina displays.
	// ignore invalid width/height values generated when minimizing the window
	if (width == 0 || height == 0 || (width == SCR_WIDTH && height == SCR_HEIGHT))
		return;
	SCR_WIDTH = width;
	SCR_HEIGHT = height;
	glfwSetWindowSize(window, SCR_WIDTH, SCR_HEIGHT);
	glDeleteFramebuffers(1, &gBuffer.buffer);
	initGBuffer();
	glViewport(0, 0, width, height);
}

void glfwErrorCallback(int errorno, const char* errmsg) {
	ERRORCOLOR(std::cout << "GLFW Error #" << errorno << ": " << errmsg << std::endl);
}

bool beginFrame(bool tickPhysics) {
	// update frame
	updateTime();
	resetSingleFrameInput();
	glfwPollEvents();
	if (glfwWindowShouldClose(window))
		return false;
	if (tickPhysics)
		updatePhysics();
	return true;
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
	GLuint quadVBO;
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glBindVertexArray(0);
	glDeleteBuffers(1, &quadVBO);
}

void renderQuad() {
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void initLightCube() {
	GLfloat cubeStripVerts[] = {
		-1.f, 1.f, 1.f,
		1.f, 1.f, 1.f,
		-1.f, -1.f, 1.f,
		1.f, -1.f, 1.f,
		1.f, -1.f, -1.f,
		1.f, 1.f, 1.f,
		1.f, 1.f, -1.f,
		-1.f, 1.f, 1.f,
		-1.f, 1.f, -1.f,
		-1.f, -1.f, 1.f,
		-1.f, -1.f, -1.f,
		1.f, -1.f, -1.f,
		-1.f, 1.f, -1.f,
		1.f, 1.f, -1.f 
	};
	glGenVertexArrays(1, &lightCubeVAO);
	GLuint lightCubeVBO;
	glGenBuffers(1, &lightCubeVBO);
	// fill buffer
	glBindBuffer(GL_ARRAY_BUFFER, lightCubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeStripVerts), cubeStripVerts, GL_STATIC_DRAW);
	// link vertex attributes
	glBindVertexArray(lightCubeVAO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glDeleteBuffers(1, &lightCubeVBO);
}

void renderLightCube() {
	// render Cube
	glBindVertexArray(lightCubeVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 14);
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

void renderText(std::string fontName, int fontSize, Shader& s, std::string text, GLfloat x, GLfloat y, GLfloat depth, GLfloat scale, glm::vec3 color, bool centered) {
	// TODO: precalculate individual VAOs in TextObjects for faster rendering
	// make sure we're using a valid font/size, and rendering a non-empty string
	if (!fonts[fontName].count(fontSize)) {
		ERRORCOLOR(std::cout << "Error: font '" << fontName << "' at size '" << fontSize << "' not found in fonts map; please load this (font,size) pair and try again" << std::endl);
		return;
	}
	if (text.length() == 0)
		return;

	// activate and prepare the shader
	glBindVertexArray(TextObject::VAO);
	glBindBuffer(GL_ARRAY_BUFFER, TextObject::VBO);
	// only rebind the font atlas if we switched fonts, font sizes, or shaders (texture binding is not tied to the active shader, so other shaders likely wrote over our texture)
	if (s.use() || renderState.lastFontUsed.first != fontName || renderState.lastFontUsed.second != fontSize) {
		renderState.lastFontUsed = { fontName, fontSize };
		glBindTexture(GL_TEXTURE_2D, fonts[fontName][fontSize].first);
	}
	if (renderState.lastFontColor != color) {
		renderState.lastFontColor = color;
		s.setVec3("textColor", color.x, color.y, color.z);
	}
	s.setFloat("depth", depth);

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
	for (unsigned int i = 0; i < text.length(); ++i) {
		ch = fonts[fontName][fontSize].second[text[i]];

		GLfloat xpos = x + ch.x_off * scale;
		GLfloat ypos = y + (fonts[fontName][fontSize].second['A'].y_size - ch.y_off) * scale;
		GLfloat w = ch.x_size * scale;
		GLfloat h = ch.y_size * scale;

		// TODO: try instanced rendering + tristrips (see GameObject2D / ParticleEmitter2D) for a minor performance boost
		// add character position and uv data to the vertex vector
		verts[24 * i]    = xpos;
		verts[24 * i+1]  = ypos;
		verts[24 * i+2]  = ch.x0;
		verts[24 * i+3]  = ch.y0;

		verts[24 * i+4]  = xpos;
		verts[24 * i+5]  = ypos + h;
		verts[24 * i+6]  = ch.x0;
		verts[24 * i+7]  = ch.y1;
		
		verts[24 * i+8]  = xpos + w;
		verts[24 * i+9]  = ypos + h;
		verts[24 * i+10] = ch.x1;
		verts[24 * i+11] = ch.y1;
		
		verts[24 * i+12]  = xpos;
		verts[24 * i+13]  = ypos;
		verts[24 * i+14] = ch.x0;
		verts[24 * i+15] = ch.y0;
		
		verts[24 * i+16]  = xpos + w;
		verts[24 * i+17]  = ypos + h;
		verts[24 * i+18] = ch.x1;
		verts[24 * i+19] = ch.y1;
		
		verts[24 * i+20] = xpos + w;
		verts[24 * i+21] = ypos;
		verts[24 * i+22] = ch.x1;
		verts[24 * i+23] = ch.y0;

		// Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		x += (ch.advance) * scale;
	}

	// render the full set of glyphs as a single triangle array
	glBufferSubData(GL_ARRAY_BUFFER, 0, 24 * sizeof(GLfloat) * text.length(), &verts[0]);
	glDrawArrays(GL_TRIANGLES, 0, text.length()*6);	
}

void freetypeLoadFont(std::string fontName, int fontSize) {
	FT_Library ft;
	FT_Face    face;

	FT_Init_FreeType(&ft);
	if (FT_New_Face(ft, (fontDir + fontName + ".ttf").c_str(), 0, &face))
		ERRORCOLOR(std::cout << "Freetype error: Failed to load font pair '" << fontName << ", " << fontSize << "'" << std::endl);
	FT_Set_Pixel_Sizes(face, 0, fontSize);

	// calculate texture width
	unsigned int max_dim = static_cast<int>((1 + (face->size->metrics.height >> 6))* ceilf(sqrtf(numFontCharacters)));
	unsigned int tex_width = 1;
	while (tex_width < max_dim)
		tex_width <<= 1;
	// tex_height is initially set to match tex_width; once we're done loading in the glyphs, we'll be able to create the final texture with the exact height
	unsigned int tex_height = tex_width;

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

		for (unsigned int row = 0; row < bmp->rows; ++row) {
			for (unsigned int col = 0; col < bmp->width; ++col) {
				int x = pen_x + col;
				int y = pen_y + row;
				pixels[y * tex_width + x] = bmp->buffer[row * bmp->pitch + col];
			}
		}

		// populate Character struct with info needed for rendering
		fonts[fontName][fontSize].second[i].x0 = static_cast<float>(pen_x);
		fonts[fontName][fontSize].second[i].y0 = static_cast<float>(pen_y);
		fonts[fontName][fontSize].second[i].x1 = static_cast<float>(pen_x + bmp->width);
		fonts[fontName][fontSize].second[i].y1 = static_cast<float>(pen_y + bmp->rows);

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
	SUCCESSCOLOR(std::cout << "Successfully loaded font pair '" << fontName << ", " << fontSize << "'" << std::endl);
}

void queueDrawPoint(glm::vec3 pos, glm::vec3 color) {
	// TODO: offset screen coords by 0.5f to draw points from pixel centers rather than corners?
	size_t qsize = pointsQueue.size();
	unsigned int vec3Size = 3 * sizeof(GLfloat);
	pointsQueue.resize(qsize + 6);
	memcpy(&pointsQueue[qsize], &pos, vec3Size);
	memcpy(&pointsQueue[qsize + 3], &color, vec3Size);
}

void drawPoints() {
	if (pointsQueue.empty())
		return;
	glBindVertexArray(primitiveVAO);
	glBindBuffer(GL_ARRAY_BUFFER, primitiveVBO);
	if (pointsQueue.size() > numPointsInVBO) {
		numPointsInVBO = pointsQueue.size();
		glBufferData(GL_ARRAY_BUFFER, pointsQueue.size() * sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW);
	}
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * pointsQueue.size(), &pointsQueue[0]);

	glDrawArrays(GL_POINTS, 0, pointsQueue.size() / 6);
	pointsQueue.clear();
}

void queueDrawLine(const glm::vec3& from, const glm::vec3& to, const glm::vec3& color) {
	// TODO: enable blending and take in vec4 color rather than vec3 for alpha support
	// TODO: offset screen coords by 0.5f to draw lines from pixel centers rather than corners?
	size_t qsize = linesQueue.size();
	unsigned int vec3Size = 3 * sizeof(GLfloat);
	linesQueue.resize(qsize + 12);
	memcpy(&linesQueue[qsize], &from, vec3Size);
	memcpy(&linesQueue[qsize+3], &color, vec3Size);
	memcpy(&linesQueue[qsize+6], &to, vec3Size);
	memcpy(&linesQueue[qsize+9], &color, vec3Size);
}

void drawLines() {
	if (linesQueue.empty())
		return;
	glBindVertexArray(primitiveVAO);
	glBindBuffer(GL_ARRAY_BUFFER, primitiveVBO);
	if (linesQueue.size() > numLinesInVBO) {
		numLinesInVBO = linesQueue.size();
		glBufferData(GL_ARRAY_BUFFER, linesQueue.size() * sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW);
	}
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * linesQueue.size(), &linesQueue[0]);
	
	glDrawArrays(GL_LINES, 0, linesQueue.size() / 6);
	linesQueue.clear();
}

void calcOrthoProjection() {
	// apply 2D projection matching target resolution to all 2d shaders
	// TODO: pillbox/letterbox different aspect ratios to fit 16:9
	OrthoProjection = glm::ortho(0.0f, static_cast<GLfloat>(TARGET_WIDTH), static_cast<GLfloat>(TARGET_HEIGHT), 0.0f, -1.0f, 1.0f);
	UIOrthoProjection = glm::ortho(0.0f, static_cast<GLfloat>(UI_TARGET_WIDTH), static_cast<GLfloat>(UI_TARGET_HEIGHT), 0.0f, -1.0f, 1.0f);
	shaders["textShader"]->use();
	shaders["textShader"]->setMat4("projection", UIOrthoProjection);
	shaders["tilemapShader"]->use();
	shaders["tilemapShader"]->setMat4("projection", OrthoProjection);
	shaders["lineShader2D"]->use();
	shaders["lineShader2D"]->setMat4("projection", OrthoProjection);
	shaders["2DShader"]->use();
	shaders["2DShader"]->setMat4("projection", OrthoProjection);
	shaders["ParticleShader"]->use();
	shaders["ParticleShader"]->setMat4("projection", OrthoProjection);
	shaders["Particle2DShader"]->use();
	shaders["Particle2DShader"]->setMat4("projection", OrthoProjection);
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
	// normal + emission color buffer
	glGenTextures(1, &gBuffer.normal);
	glBindTexture(GL_TEXTURE_2D, gBuffer.normal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
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
		ERRORCOLOR(std::cout << "Framebuffer not complete!" << std::endl);
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
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
		// attach depth texture as FBO's depth buffer
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO[i]);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap[i], 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}

GLFWwindow* initWindow() {
	// init glfw
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// get monitor dimensions
	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	MONITOR_WIDTH = mode->width;
	MONITOR_HEIGHT = mode->height;
	MONITOR_REFRESH_RATE = mode->refreshRate;

	// setup windows console colors here since the original console color doesn't appear to be accessible prior to main
#ifdef _WIN32
	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	GetConsoleScreenBufferInfo(hConsole, &cbInfo);
	originalColor = cbInfo.wAttributes;
#endif

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// create the game window
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "RaiderEngine", NULL, NULL);
	if (window == NULL) {
		ERRORCOLOR(std::cout << "Failed to create GLFW window" << std::endl);
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	if (renderState.fullScreen)
		setWindowMode(SCR_WIDTH, SCR_HEIGHT, true);

	// callback events
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, frameBufferSizeCallback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetKeyCallback(window, key_callback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, renderState.enableCursor ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);

	glfwSwapInterval(renderState.useVsync);

	return window;
}

void initGL() {
	// load opengl function pointers
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		ERRORCOLOR(std::cout << "Failed to initialize GLAD" << std::endl);
			exit(EXIT_FAILURE);
	}

	// enable line/point rendering
	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
	glEnable(GL_PROGRAM_POINT_SIZE);
}

void loadShaders() {
	// load shaders
	shaders["shaderGeometryPass"] = std::make_unique<Shader>("g_buffer.vert", "g_buffer.frag");
	shaders["shaderLightingPass"] = std::make_unique<Shader>("deferred_shading.vert", "deferred_shading.frag");
	shaders["lightCube"] = std::make_unique<Shader>("lightCube.vert", "lightCube.frag");
	shaders["lineShader"] = std::make_unique<Shader>("lineShader.vert", "lineShader.frag");
	shaders["lineShader2D"] = std::make_unique<Shader>("lineShader2D.vert", "lineShader2D.frag");
	shaders["textShader"] = std::make_unique<Shader>("textShader.vert", "textShader.frag");
	shaders["tilemapShader"] = std::make_unique<Shader>("tilemapShader.vert", "tilemapShader.frag");
	shaders["2DShader"] = std::make_unique<Shader>("2DShader.vert", "2DShader.frag");
	shaders["ParticleShader"] = std::make_unique<Shader>("ParticleShader.vert", "ParticleShader.frag");
	shaders["Particle2DShader"] = std::make_unique<Shader>("Particle2DShader.vert", "Particle2DShader.frag");
	shaders["pointShadowsDepth"] = std::make_unique<Shader>("point_shadows_depth.vert", "point_shadows_depth.frag", "point_shadows_depth.geom");

	// configure shaders
	shaders["shaderLightingPass"]->use();
	shaders["shaderLightingPass"]->setInt("gPosition", 0);
	shaders["shaderLightingPass"]->setInt("gNormal", 1);
	shaders["shaderLightingPass"]->setInt("gAlbedoSpec", 2);
	shaders["shaderLightingPass"]->setInt("depthMap0", 3);
	shaders["shaderLightingPass"]->setInt("depthMap1", 4);
	shaders["shaderLightingPass"]->setInt("depthMap2", 5);
	shaders["shaderLightingPass"]->setInt("depthMap3", 6);
	shaders["shaderLightingPass"]->setInt("depthMap4", 7);

	shaders["2DShader"]->use();
	shaders["2DShader"]->setInt("image", 0);

	shaders["lineShader"]->use();
	glBindVertexArray(primitiveVAO);
	glBindBuffer(GL_ARRAY_BUFFER, primitiveVBO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
}

void drawGameObjects(std::string shaderName, int shouldSendTextures, bool ignoreNonShadowCasters) {
	// TODO: consider caching model matrix array / buffered VAO data for static GameObjects
	glBindBuffer(GL_ARRAY_BUFFER, GameObject::instancedModelVBO);
	for (auto&& kv : gameObjects) {
		if (kv.second.size() == 0)
			continue;
		if (ignoreNonShadowCasters && !kv.second[0]->castShadows)
			continue;
		// toggle face culling when drawing a double-sided object
		if (kv.second[0]->drawTwoSided != renderState.twoSidedDrawing) {
			if (kv.second[0]->drawTwoSided)
				glDisable(GL_CULL_FACE);
			else
				glEnable(GL_CULL_FACE);
			renderState.twoSidedDrawing = !renderState.twoSidedDrawing;
		}
		// iterate through all submeshes
		for (unsigned int i = 0; i < kv.second[0]->model->meshes.size(); ++i) {
			glBindVertexArray(*kv.second[0]->model->meshes[i].VAO);
			// transfer the textures for the current submesh once initially
			if (shouldSendTextures > 0)
				kv.second[0]->model->meshes[i].sendTexturesToShader(*shaders[shaderName], shouldSendTextures == 1);
			// now render the current submesh for all GameObjects
			if (kv.second.size() > modelMatrices3D.capacity()) {
				modelMatrices3D.reserve(kv.second.size());
				glBufferData(GL_ARRAY_BUFFER, kv.second.size() * sizeof(glm::mat4), NULL, GL_DYNAMIC_DRAW);
			}
			for (unsigned int r = 0; r < kv.second.size(); ++r) {
				if (kv.second[r]->isDirty)
					kv.second[r]->recalculateModelTransform();
				modelMatrices3D[r] = kv.second[r]->modelTransform;
			}
			glBufferSubData(GL_ARRAY_BUFFER, 0, kv.second.size() * sizeof(glm::mat4), &modelMatrices3D[0]);
			glDrawElementsInstanced(GL_TRIANGLES, kv.second[0]->model->meshes[i].indices.size(), GL_UNSIGNED_INT, 0, kv.second.size());			
		}
	}
}

void renderDepthMap() {
	// create depth cubemap transformation matrices
	setShouldBlend(true);
	setShouldDepthTest(true);
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	shaders["pointShadowsDepth"]->use();
	if (mainCam->far_plane != renderState.far_plane)
		shaders["pointShadowsDepth"]->setFloat("far_plane", mainCam->far_plane);
	glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT, mainCam->near_plane, mainCam->far_plane);
	for (unsigned int k = 0; k < lights.size(); ++k) {
		if (lights[k]->on) {
			glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO[k]);
			glClear(GL_DEPTH_BUFFER_BIT);
			glm::vec3 lightPos = lights[k]->position;
			std::vector<glm::mat4> shadowTransforms {
			shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
			shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
			shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
			shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
			shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
			shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)) };

			// render scene to depth cubemap
			for (unsigned int i = 0; i < 6; ++i)
				shaders["pointShadowsDepth"]->setMat4("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
			shaders["pointShadowsDepth"]->setVec3("lightPos", lightPos);
			drawGameObjects("pointShadowsDepth", 1, true);
		}
	}
}

void renderGeometryPass() {
	setGlViewport();
	setShouldBlend(false);
	// geometry pass: render scene's geometry/color data into gbuffer
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer.buffer);
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	shaders["shaderGeometryPass"]->use();
	if (mainCam->projection != renderState.projection)
		shaders["shaderGeometryPass"]->setMat4("projection", mainCam->projection);
	if (mainCam->view != renderState.view)
		shaders["shaderGeometryPass"]->setMat4("view", mainCam->view);
	if (mainCam->Position != renderState.viewPos)
		shaders["shaderGeometryPass"]->setVec3("viewPos", mainCam->Position);
	shaders["shaderGeometryPass"]->setFloat("Time", (float)totalTime);
	drawGameObjects("shaderGeometryPass");

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void renderLightingPass() {
	// lighting pass: calculate lighting by iterating over a screen filled quad pixel-by-pixel using the gbuffer's content.
	glClearColor(renderState.clearColor.r * renderState.ambientStrength, renderState.clearColor.g * renderState.ambientStrength, renderState.clearColor.b * renderState.ambientStrength, renderState.clearColor.a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	shaders["shaderLightingPass"]->use();
	if (renderState.clearColor != renderState.prevClearColor)
		shaders["shaderLightingPass"]->setVec4("clearColor", renderState.clearColor);
	if (renderState.ambientStrength != renderState.prevAmbientStrength)
		shaders["shaderLightingPass"]->setFloat("ambientStrength", renderState.ambientStrength);
	if (renderState.underWater != PlayerSpawn::player->underWater)
		shaders["shaderLightingPass"]->setBool("underWater", PlayerSpawn::player->underWater);

	glBindTexture(GL_TEXTURE_2D, gBuffer.position);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gBuffer.normal);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, gBuffer.albedoSpec);
	// send light relevant uniforms
	if (renderState.numLights != lights.size())
		shaders["shaderLightingPass"]->setInt("NR_LIGHTS", enableLighting ? lights.size() : 0);

	for (unsigned int i = 0; i < lights.size(); ++i) {
		if (lights[i]->dirty) {
			lights[i]->dirty = false;
			// TODO: cascade dirty flag when removing a light from the scene
			if (lights[i]->on) {
				// NOTE: consider further breaking down dirty state on a per-attribute basis
				shaders["shaderLightingPass"]->setVec3("lights[" + std::to_string(i) + "].Position", lights[i]->position);
				shaders["shaderLightingPass"]->setVec3("lights[" + std::to_string(i) + "].Color", lights[i]->color);
				shaders["shaderLightingPass"]->setFloat("lights[" + std::to_string(i) + "].Linear", lights[i]->linear);
				shaders["shaderLightingPass"]->setFloat("lights[" + std::to_string(i) + "].Quadratic", lights[i]->quadratic);
				shaders["shaderLightingPass"]->setFloat("lights[" + std::to_string(i) + "].Radius", lights[i]->radius);
				shaders["shaderLightingPass"]->setFloat("lights[" + std::to_string(i) + "].EnableShadows", lights[i]->enableShadows);
			}
			shaders["shaderLightingPass"]->setFloat("lights[" + std::to_string(i) + "].On", lights[i]->on);
		}
	}
	
	// shadow and lighting uniforms
	if (mainCam->Position != renderState.viewPos)
		shaders["shaderLightingPass"]->setVec3("viewPos", mainCam->Position);
	if (mainCam->far_plane != renderState.far_plane)
		shaders["shaderLightingPass"]->setFloat("far_plane", mainCam->far_plane);
	
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap[0]);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap[1]);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap[2]);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap[3]);

	// reactivate tex0 as this is the assumed state
	glActiveTexture(GL_TEXTURE0);

	// finally, render everything to the screen as a single quad
	renderQuad();

	// copy content of geometry's depth buffer to default framebuffer's depth buffer
	glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer.buffer);
	glBlitFramebuffer(0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0, SCR_WIDTH, SCR_HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
}

void renderParticles() {
	// render Particles
	glDepthMask(GL_FALSE);

	shaders["ParticleShader"]->use();
	shaders["ParticleShader"]->setMat4("view", mainCam->view);
	glBindVertexArray(ParticleEmitter::VAO);
	glBindBuffer(GL_ARRAY_BUFFER, ParticleEmitter::VBO);
	setBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);
	for (int i = 0; i < 2; setBlendFunc(GL_SRC_ALPHA, GL_ONE), ++i) {
		// render particles in two passes; once in black, and once normally. Results in additive blending between particles, and normal blending with everything else
		// TODO: there may be a way to optimize this. Consider rendering to a second buffer rather than doing two passes, or possible single-pass solution using premultiplied alpha
		for (auto&& pe : particleEmitters) {
			glBindTexture(GL_TEXTURE_2D, pe->sprite.id);
			shaders["ParticleShader"]->setVec2("spriteDims", glm::vec2(pe->sprite.width, pe->sprite.height));
			if (pe->particles.size() > ParticleEmitter::numParticlesInVBO) {
				ParticleEmitter::numParticlesInVBO = pe->particles.size();
				glBufferData(GL_ARRAY_BUFFER, pe->particles.size() * 8 * sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW);
			}
			glBufferSubData(GL_ARRAY_BUFFER, 0, pe->particles.size() * 8 * sizeof(GLfloat), &pe->particles[0]);
			glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, pe->particles.size());
		}
	}
	glDepthMask(GL_TRUE);
}

void debugDrawLightCubes() {
	if (drawLightCubes) {
		shaders["lightCube"]->use();
		shaders["lightCube"]->setMat4("projection", mainCam->projection);
		shaders["lightCube"]->setMat4("view", mainCam->view);
	}
	if (drawLightCubes || drawLightSpheres) {
		for (unsigned int i = 0; i < lights.size(); ++i) {
			if (drawLightCubes) {
				// render small solid cube at light position
				shaders["lightCube"]->setMat4("model", glm::scale(glm::translate(glm::mat4(1.0f), lights[i]->position), glm::vec3(lights[i]->radius*.005f)));
				shaders["lightCube"]->setVec3("lightColor", lights[i]->on ? lights[i]->color : lights[i]->offColor);
				renderLightCube();
			}
			if (drawLightSpheres) {
				// render wireframe sphere showing light radius
				#define numSegs 12
				#define PI glm::pi<float>()
				#define TAU glm::two_pi<float>()
				glm::vec3 vertPositions[numSegs * numSegs];
				for (int k = 0; k < numSegs; ++k) {
					for (int r = 0; r < numSegs; ++r) {
						glm::vec2 seg(k / static_cast<float>(numSegs-1), r / static_cast<float>(numSegs-1));
						vertPositions[k * numSegs + r] = lights[i]->position + lights[i]->radius * glm::vec3(
							std::cos(seg.x * TAU) * std::sin(seg.y * PI),
							std::cos(seg.y * PI), 
							std::sin(seg.x * TAU) * std::sin(seg.y * PI));
					}
				}
				for (int k = 0; k < numSegs; ++k) {
					for (int r = 0; r < numSegs; ++r) {
						if (r != numSegs-1)
							queueDrawLine(vertPositions[k * numSegs + r], vertPositions[k * numSegs + (r+1)%numSegs], Color::yellow);
						queueDrawLine(vertPositions[k * numSegs + r], vertPositions[((k+1)%numSegs) * numSegs + r], Color::yellow);
					}
				}
			}
		}
	}
}

void renderLines() {
	// TODO: distinguish between queuing 3d lines and points to be rendered, and 2d lines and points to be rendered
	// draw 3d colliders
	if (drawColliders)
		debugDrawPhysics();
	// draw normals
	if (drawNormals) {
		glm::vec4 vertP, vertE;
		for (auto&& kv : gameObjects) {
			if (kv.second.size() == 0)
				continue;
			for (unsigned int i = 0; i < kv.second[0]->model->meshes.size(); ++i) {
				for (const Vertex& v : kv.second[0]->model->meshes[i].vertices) {
					vertP = glm::vec4(v.Position, 1);
					for (unsigned int r = 0; r < kv.second.size(); ++r) {
						// scale the normal magnitude by the inverse of the model scale to cancel out the scaling factor in the model transform
						vertE = vertP + glm::vec4(v.Normal * .05f * (3.f / (kv.second[r]->scaleVal.x + kv.second[r]->scaleVal.y + kv.second[r]->scaleVal.z)), 0);
						queueDrawLine(kv.second[r]->modelTransform * vertP, kv.second[r]->modelTransform * vertE, Color::indigo);
					}
				}
			}
		}
	}
	if (!pointsQueue.empty() || !linesQueue.empty()) {
		// setup
		if (shaders["lineShader"]->use()) {
			setShouldDepthTest(false);
			if (mainCam->projection != renderState.projection)
				shaders["lineShader"]->setMat4("projection", mainCam->projection);
			if (mainCam->view != renderState.view)
				shaders["lineShader"]->setMat4("view", mainCam->view);
		}
		drawPoints();
		drawLines();
	}
}

void renderLines2D() {
	// TODO: enable rendering 2d points
	if (drawColliders) {
		// draw 2d colliders
		for (auto&& kv : gameObject2Ds)
			for (unsigned int i = 0; i < kv.second.size(); ++i)
				if (kv.second[i]->collider)
					kv.second[i]->collider->debugDraw(kv.second[i]->center, kv.second[i]->rotation);
		for (auto& t : tilemaps)
			for (unsigned int i = 0; i < t->mapSize.x; ++i)
				for (unsigned int r = 0; r < t->mapSize.y; ++r)
					if (t->tileColliders[t->map[i][r]])
						t->tileColliders[t->map[i][r]]->debugDraw(glm::vec2(t->pos.x + t->gridSize / 2.f + t->gridSize * i, t->pos.y + t->gridSize / 2.f + t->gridSize * r), 0);
	}
	if (!linesQueue.empty()) {
		// setup
		if (shaders["lineShader2D"]->use())
			setShouldDepthTest(false);
		drawLines();
	}
}

void setGlViewport() {
	// TODO: store in renderState
	float ratx = SCR_WIDTH / static_cast<float>(TARGET_WIDTH), raty = SCR_HEIGHT / static_cast<float>(TARGET_HEIGHT);
	if (ratx == raty)
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
	else if (ratx > raty)
		glViewport(static_cast<GLint>(.5f * (SCR_WIDTH - SCR_WIDTH * (raty / ratx))), 0, static_cast<GLint>(SCR_WIDTH * (raty / ratx)), SCR_HEIGHT);
	else
		glViewport(0, static_cast<GLint>(.5f * (SCR_HEIGHT - SCR_HEIGHT * (ratx / raty))), SCR_WIDTH, static_cast<GLint>(SCR_HEIGHT * (ratx / raty)));
}

void render2D(bool clearScreen) {
	setGlViewport();
	setShouldDepthTest(true);
	// clear the depth buffer so 2D objects don't compete with 3d objects for visibility
	glClear(GL_DEPTH_BUFFER_BIT | (clearScreen ? GL_COLOR_BUFFER_BIT : 0));
	setShouldBlend(true);
	setBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// render Tilemaps
	shaders["tilemapShader"]->use();
	for (unsigned int i = 0; i < tilemaps.size(); ++i) {
		glBindVertexArray(tilemaps[i]->VAO);
		shaders["tilemapShader"]->setVec2("pos", tilemaps[i]->pos);
		shaders["tilemapShader"]->setFloat("depth", tilemaps[i]->depth);
		glBindBuffer(GL_ARRAY_BUFFER, tilemaps[i]->VBO);
		glBindTexture(GL_TEXTURE_2D, tilemaps[i]->sprite.id);
		glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(tilemaps[i]->mapSize.x * tilemaps[i]->mapSize.y * 6));
	}

	// render GameObject2Ds
	shaders["2DShader"]->use();
	shaders["2DShader"]->setFloat("Time", (float)totalTime);
	glBindVertexArray(GameObject2D::VAO);
	for (auto&& kv : gameObject2Ds) {
		// don't render gameObjects using the default (empty) sprite
		if (kv.second[0]->sprite.id == Model::defaultDiffuseMap.id)
			continue;
		if (kv.second[0]->sprite.scrollSpeed != renderState.uvScroll2D) {
			renderState.uvScroll2D = kv.second[0]->sprite.scrollSpeed;
			shaders["2DShader"]->setVec2("uvScroll", renderState.uvScroll2D.x, renderState.uvScroll2D.y);
		}
		glBindTexture(GL_TEXTURE_2D, kv.second[0]->sprite.id);
		// copy GameObject2D model matrices before rendering them all in one go
		// TODO: consider caching model matrix array / buffered VAO data for static GameObject2Ds
		if (kv.second.size() > modelMatrices2D.capacity()) {
			modelMatrices2D.reserve(kv.second.size());
			colorVectors.reserve(kv.second.size());
			glBindBuffer(GL_ARRAY_BUFFER, GameObject2D::instancedModelVBO);
			glBufferData(GL_ARRAY_BUFFER, kv.second.size() * sizeof(glm::mat4), NULL, GL_DYNAMIC_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, GameObject2D::instancedColorVBO);
			glBufferData(GL_ARRAY_BUFFER, kv.second.size() * sizeof(glm::vec3), NULL, GL_DYNAMIC_DRAW);
		}
		for (unsigned int i = 0; i < kv.second.size(); ++i) {
			if (kv.second[i]->isDirty)
				kv.second[i]->recalculateModel();
			modelMatrices2D[i] = kv.second[i]->model;
			colorVectors[i] = kv.second[i]->color;
		}
		glBindBuffer(GL_ARRAY_BUFFER, GameObject2D::instancedModelVBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, kv.second.size() * sizeof(glm::mat4), &modelMatrices2D[0]);
		glBindBuffer(GL_ARRAY_BUFFER, GameObject2D::instancedColorVBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, kv.second.size() * sizeof(glm::vec3), &colorVectors[0]);
		glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, kv.second.size());
	}

	// render Particle2Ds
	glDepthMask(GL_FALSE);

	shaders["Particle2DShader"]->use();
	glBindVertexArray(ParticleEmitter2D::VAO);
	glBindBuffer(GL_ARRAY_BUFFER, ParticleEmitter2D::VBO);
	setBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);
	for (int i = 0; i < 2; setBlendFunc(GL_SRC_ALPHA, GL_ONE), ++i) {
		// render particles in two passes; once in black, and once normally. Results in additive blending between particles, and normal blending with everything else
		// TODO: there may be a way to optimize this. Consider rendering to a second buffer rather than doing two passes, or possible single-pass solution using premultiplied alpha
		for (auto&& pe : particleEmitter2Ds) {
			glBindTexture(GL_TEXTURE_2D, pe->sprite.id);
			shaders["Particle2DShader"]->setVec2("spriteDims", glm::vec2(pe->sprite.width, pe->sprite.height));
			shaders["Particle2DShader"]->setFloat("depth", pe->depth);
			if (pe->particles.size() > ParticleEmitter2D::numParticlesInVBO) {
				ParticleEmitter2D::numParticlesInVBO = pe->particles.size();
				glBufferData(GL_ARRAY_BUFFER, pe->particles.size() * 7 * sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW);
			}
			glBufferSubData(GL_ARRAY_BUFFER, 0, pe->particles.size() * 7 * sizeof(GLfloat), &pe->particles[0]);
			glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, pe->particles.size());
		}
	}

	// render text
	setBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	for (unsigned int i = 0; i < textObjects.size(); ++i)
		textObjects[i]->draw(*shaders["textShader"]);
	glDepthMask(GL_TRUE);
}

void render(bool only2D) {
	double sTime = glfwGetTime();
	if (!only2D) {
		renderDepthMap();
		renderGeometryPass();
		renderLightingPass();
		renderParticles();
		debugDrawLightCubes();
		renderLines();
	}
	
	render2D(only2D);
	renderLines2D();
	glfwSwapBuffers(window);

	// update render state
	renderState.prevAmbientStrength = renderState.ambientStrength;
	renderState.prevClearColor = renderState.clearColor;
	renderState.underWater = PlayerSpawn::player && PlayerSpawn::player->underWater;
	renderState.projection = mainCam->projection;
	renderState.view = mainCam->view;
	renderState.viewPos = mainCam->Position;
	renderState.numLights = lights.size();
	renderState.far_plane = mainCam->far_plane;

	frameRenderTime = glfwGetTime() - sTime;
}

void initMainCamera() {
	mainCam = new Camera(glm::vec3(0));
}

void NVSettingsCheckError(NvAPI_Status status) {
	if (status == NVAPI_OK)
		return;

	NvAPI_ShortString szDesc = { 0 };
	NvAPI_GetErrorMessage(status, szDesc);
	ERRORCOLOR(printf("NVAPI error: %s\n", szDesc));
	exit(-1);
}


void NVSettingsSetString(NvAPI_UnicodeString& nvStr, const wchar_t* wcStr) {
	for (int i = 0; (nvStr[i] = wcStr[i]) != 0 && i < NVAPI_UNICODE_STRING_MAX; ++i);
}


void checkDisableNvidiaThreadedOptimization() {
	// TODO: investigate the cause of microstutters while nvidia threaded optimization is enabled. disabling it may not be necessary (and restarting the application is not ideal)
	if (std::string(reinterpret_cast<char const*>(glGetString(GL_VENDOR))) == std::string("NVIDIA Corporation")) {
		std::cout << "Nvidia gpu detected; checking threaded optimization status" << std::endl;
		// profile settings / initialization
		const wchar_t* profileName = L"RaiderEngine_NoThreadedOptimization";
		const wchar_t* appName = L"RaiderEngine.exe";
		const wchar_t* appFriendlyName = L"Raider Engine";
		NvDRSSessionHandle hSession;
		NVSettingsCheckError(NvAPI_Initialize());
		NVSettingsCheckError(NvAPI_DRS_CreateSession(&hSession));
		NVSettingsCheckError(NvAPI_DRS_LoadSettings(hSession));

		// fill Profile Info
		NVDRS_PROFILE profileInfo;
		profileInfo.version = NVDRS_PROFILE_VER;
		profileInfo.isPredefined = 0;
		NVSettingsSetString(profileInfo.profileName, profileName);

		// initial setting configuration
		NVDRS_SETTING setting;
		setting.version = NVDRS_SETTING_VER;
		NvDRSProfileHandle hProfile;
		
		if (NvAPI_DRS_FindProfileByName(hSession, profileInfo.profileName, &hProfile) == NVAPI_OK) {
			// profile already exists
			NVSettingsCheckError(NvAPI_DRS_GetSetting(hSession, hProfile, OGL_THREAD_CONTROL_ID, &setting));
			if (setting.u32CurrentValue == OGL_THREAD_CONTROL_DISABLE) {
				// threaded optimization is already disabled
				std::cout << "profile already exists with threaded optimization disabled; nothing more to do" << std::endl;
				NvAPI_DRS_DestroySession(hSession);
				return;
			}
			// profile exists with incorrect threading optimization setting
			std::cout << "profile already exists with incorrect threaded optimization setting; disabling it now" << std::endl;
			goto configureSetting;
		}

		// no profile exists; create a new one
		NVSettingsCheckError(NvAPI_DRS_CreateProfile(hSession, &profileInfo, &hProfile));

		// configure application Info
		NVDRS_APPLICATION app;
		app.version = NVDRS_APPLICATION_VER_V1;
		app.isPredefined = 0;
		NVSettingsSetString(app.appName, appName);
		NVSettingsSetString(app.userFriendlyName, appFriendlyName);
		NVSettingsSetString(app.launcher, L"");
		NVSettingsSetString(app.fileInFolder, L"");

		// create application
		NVSettingsCheckError(NvAPI_DRS_CreateApplication(hSession, hProfile, &app));

		// configure setting
	configureSetting:
		setting.settingId = OGL_THREAD_CONTROL_ID;
		setting.settingType = NVDRS_DWORD_TYPE;
		setting.settingLocation = NVDRS_CURRENT_PROFILE_LOCATION;
		setting.isCurrentPredefined = 0;
		setting.isPredefinedValid = 0;
		setting.u32CurrentValue = OGL_THREAD_CONTROL_DISABLE;
		setting.u32PredefinedValue = OGL_THREAD_CONTROL_DISABLE;

		// set setting and save it
		NVSettingsCheckError(NvAPI_DRS_SetSetting(hSession, hProfile, &setting));
		NVSettingsCheckError(NvAPI_DRS_SaveSettings(hSession));

		NvAPI_DRS_DestroySession(hSession);
		std::cout << "successfully created or updated nvidia application profile with threaded optimization disabled. Pease restart the application now" << std::endl;
		exit(EXIT_SUCCESS);
	}
}

void initGraphics() {
	renderState.clearColor.a = 1;
	glfwSetErrorCallback(glfwErrorCallback);
	window = initWindow();
	if (forceDisableNvidiaThreadedOptimization)
		checkDisableNvidiaThreadedOptimization();
	initGL();
	initQuad();
	initLightCube();
	initGBuffer();
	initDepthMaps();
	initPhysics();
	initBuffers();
	initMainCamera();
	loadShaders();
	calcOrthoProjection();
	Model::createDefaultMaterialMaps();
	GameObject::initStaticVertexBuffer();
	GameObject2D::initStaticVertexBuffer();
	ParticleEmitter2D::initVertexObjects();
	TextObject::initVertexObjects();
}

void closeGraphics() {
	glfwTerminate();
}