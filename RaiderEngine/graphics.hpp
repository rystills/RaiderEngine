#pragma once
#include "stdafx.h"
#include "shader.hpp"

// this file is responsible for graphics, including debug and text rendering
inline unsigned int primitiveVBO, primitiveVAO;
inline unsigned int textVBO, textVAO;

#define NR_LIGHTS 4
inline unsigned int depthMapFBO[NR_LIGHTS];
inline unsigned int depthCubemap[NR_LIGHTS];
inline glm::vec4 clearColor(0,0,0,1);

inline std::vector<GLfloat> pointsQueue;
inline std::vector<GLfloat> linesQueue;
inline bool twoSidedDrawing = true;
inline float ambientStrength = .3f;

inline unsigned int quadVAO;
inline unsigned int quadVBO;

inline unsigned int cubeVAO;
inline unsigned int cubeVBO;

inline glm::vec3 stateColors[3] = { glm::vec3(255,0,0), glm::vec3(0,0,255), glm::vec3(255,255,255) };

// vectors containing model matrices and sprite colors, to be sent to the gpu for instanced rendering; grows as necessary
inline std::vector<glm::mat4> modelMatrices(0);
inline std::vector<glm::vec3> colorVectors(0);

inline glm::highp_mat4 glmOrthoProjection;
inline glm::highp_mat4 glmOrthoTextProjection;

struct GBuffer {
	unsigned int buffer, position, normal, albedoSpec;
} inline gBuffer;

struct Character {
	float x0, y0, x1, y1;  // coords of glyph in the texture atlas, baked in uv-space
	int x_off, y_off;    // left & top bearing when rendering
	int x_size, y_size;  // total glyph size, equal to (x1,y1) - (x0,y0)
	int advance;         // x advance when rendering
};

// callback executed on window resize
void frameBufferSizeCallback(GLFWwindow* window, int width, int height);

void glfwErrorCallback(int errorno, const char* errmsg);

void initQuad();

/*
render a 1x1 XY quad in NDC
*/
void renderQuad();

void initCube();

/*
render a 1x1 3D cube in NDC
*/
void renderCube();

// enable anisotropic filtering for all textures, if supported on the user's graphics card
void applyAnisotropicFiltering();

/*
initialize the default vertex buffer and attribute objects (VBO and VAO, respectively)
*/
void initBuffers();

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
@param shouldUseShader: whether or not we need to use the shader before rendering the text
*/
void renderText(std::string fontName, int fontSize, Shader& s, std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color, bool centered = false, bool shouldUseShader = true);
/*
load the specified font at the desired size using freetype, adding the (fontName,size) pair to the fonts map
@param fontName: the name of the font to load
@param fontSize: the size at which to load the font
*/
void freetypeLoadFont(std::string fontName, int fontSize);

/*
initialize freetype, creating a VBO and VAO specifically for text rendering
*/
void initFreetype();

/*
add a single point to the point vector
@param pos: the point's position
@param color: the point's rgb color
*/
void queueDrawPoint(glm::vec3 pos, glm::vec3 color);

/*
draw all of the buffered points, then clear the point vector
*/
void drawPoints();

/*
add a single line to the line vector
@param from: the line's start position
@param to: the line's end position
@param color: the line's rgb color
*/
void queueDrawLine(const glm::vec3& from, const glm::vec3& to, const glm::vec3& color);

/*
draw all of the buffered lines, then clear the line vector
*/
void drawLines();

/*
calculate the glm orthographic projection for the current screen resolution - used when rendering
*/
void calcOrthoProjection();

/*
initialize the contents of the g buffer used for deferred rendering
*/
void initGBuffer();

/*
initialize depth maps for the defined number of lights (used for rendering shadows)
*/
void initDepthMaps();

/*
initialize our game window, creating the window itself and setting input callbacks
*/
GLFWwindow* initWindow();

/*
load all engine default shaders and populate them as necessary
*/
void loadShaders();

/*
draw all GameObjects to the specified shader
@param shaderName: the shader to which to render the GameObjects
@param shouldSendTextures: whether or not to render with textures (performance gain by disabling this when generating the depthMap) 
@param ignoreNonShadowCasters: whether or not to skip GameObjects that are set to not cast shadows
*/
void drawGameObjects(std::string shaderName, bool shouldSendTextures = true, bool ignoreNonShadowCasters = false);

/*
render the depth information from each of NR_LIGHTS to the scene
*/
void renderDepthMap();

/*
render the scene geometry to the gbuffer
*/
void renderGeometryPass();

/*
render the geometry in the gbuffer, along with lighting information, to the screen framebuffer
*/
void renderLightingPass();

/*
render colored, unlit cubes indicating the position of lights in the scene
*/
void debugDrawLightCubes();

/*
configure the line shader, and draw physics wireframes (if specified)
*/
void renderLines();

void renderLines2D();

/*
draw all 2D elements, including TextObjects and GameObject2Ds
*/
void render2D(bool clearScreen = false);

/*
initialize the main camera
*/
void initMainCamera();

/*
call all of the graphics initialization steps in order
@returns the newly created GLFWwindow pointer
*/
GLFWwindow* initGraphics();