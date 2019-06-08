// terminal colors
#include <stdio.h>
#ifdef _WIN32
#include <windows.h>
#include <direct.h>
CONSOLE_SCREEN_BUFFER_INFO cbInfo;
HANDLE hConsole;
int originalColor;
#define WARNING(msg) { SetConsoleTextAttribute(hConsole, 14); msg; SetConsoleTextAttribute(hConsole, originalColor); }
#define ERROR(msg) { SetConsoleTextAttribute(hConsole, 12); msg; SetConsoleTextAttribute(hConsole, originalColor); }
#define SUCCESS(msg) { SetConsoleTextAttribute(hConsole, 10); msg; SetConsoleTextAttribute(hConsole, originalColor); }
#else
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_RESET   "\x1b[0m"
#define WARNING(msg) { printf(ANSI_COLOR_YELLOW); msg; printf(ANSI_COLOR_RESET); }
#define ERROR(msg) { printf(ANSI_COLOR_RED); msg; printf(ANSI_COLOR_RESET); }
#define SUCCESS(msg) { printf(ANSI_COLOR_GREEN); msg; printf(ANSI_COLOR_RESET); }
#endif
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define GL_TEXTURE_MAX_ANISOTROPY_EXT 0x84FE
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
unsigned int aiMapProcessFlags =
	aiProcess_CalcTangentSpace | // calculate tangents and bitangents if possible
	aiProcess_JoinIdenticalVertices | // join identical vertices/ optimize indexing
	aiProcess_Triangulate | // Ensure all verticies are triangulated (each 3 vertices are triangle)
	aiProcess_ImproveCacheLocality | // improve the cache locality of the output vertices
	aiProcess_RemoveRedundantMaterials | // remove redundant materials
	aiProcess_FindInvalidData | // detect invalid model data, such as invalid normal vectors
	aiProcess_GenUVCoords | // convert spherical, cylindrical, box and planar mapping to proper UVs
	aiProcess_TransformUVCoords | // preprocess UV transformations (scaling, translation ...)
	aiProcess_FindInstances | // search for instanced meshes and remove them by references to one master
	aiProcess_LimitBoneWeights | // limit bone weights to 4 per vertex
	aiProcess_OptimizeMeshes | // join small meshes, if possible;
	aiProcess_SplitByBoneCount | // split meshes with too many bones. Necessary for our (limited) hardware skinning shader
	0;
unsigned int aiModelProcessFlags = aiMapProcessFlags | aiProcess_PreTransformVertices; // models should not import with nonstandard transforms; bake the transform instead

#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionShapes/btShapeHull.h>
#include "filesystem.hpp"
#include "shader.hpp"
#include "camera.hpp"
#include <iostream>
#include <unordered_map>
#include <memory>

struct BulletData {
	btDiscreteDynamicsWorld* dynamicsWorld;
	btAlignedObjectArray<btCollisionShape*> collisionShapes;
	btSequentialImpulseConstraintSolver* solver;
	btBroadphaseInterface* overlappingPairCache;
	btCollisionDispatcher* dispatcher;
	btDefaultCollisionConfiguration* collisionConfiguration;
} bulletData;

float anisoFilterAmount = 0.0f;
#include "model.hpp"
std::unordered_map<std::string, std::shared_ptr<Model>> models;
#include "GameObject.hpp"
#include "Light.hpp"
std::vector<std::unique_ptr<GameObject>> gameObjects;
std::vector<std::unique_ptr<Light>> lights;

// settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// camera
Camera camera(glm::vec3(0.0f, 1.0f, 3.0f));
// mouse
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
bool firstMouse = true;
bool mousePressedLeft = false;  // whether or not the left mouse button was just pressed
bool mouseHeldLeft = false;  // whether or not the left mouse button is currently being held down
bool mouseReleasedLeft = false;  // whether or not the left mouse button was just released 
bool mousePressedRight = false;  // whether or not the right mouse button was just pressed
bool mouseHeldRight = false;  // whether or not the right mouse button is currently being held down
bool mouseReleasedRight = false;  // whether or not the right mouse button was just released 

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

#include "ObjectRegistry.hpp"
#include "inputUtils.hpp"
#include "renderUtils.hpp"

#include "ft2build.h"
#include FT_FREETYPE_H
#define numFontCharacters 128

unsigned int VBO, VAO;
unsigned int textVBO, textVAO;
#define useVsync true
#define fullScreen false

/*
update deltaTime based on the amount of time elapsed since the previous frame
*/
void updateTime() {
	float currentFrame = glfwGetTime();
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;
}

struct ProcessObjectProperties {
	glm::vec3 pos, rot, scale, geoPos;
	std::string fullName, prevName;
} tempProp;

struct GBuffer {
	unsigned int buffer, position, normal, albedoSpec;
} gBuffer;

/*
extract the base mesh name from an assimp node name, removing $_transform information, trailing numbers, and constructor arguments
@param fullName: the node name provided by assimp that we wish to strip
@returns: a stripped version of the input node name with trailing transform and numbering info removed
*/
std::string stripNodeName(std::string fullName) {
	std::size_t nameExtraStart = fullName.find("_$Assimp");
	std::size_t underscorePos = fullName.find("_");
	// fund underscore position, ignoring underscores added in by ASSIMP
	int sPos = (underscorePos == std::string::npos || (underscorePos != fullName.length()-1 && fullName[underscorePos+1] == '$') ? 0 : underscorePos+1);
	std::string name = nameExtraStart == std::string::npos ? fullName.substr(sPos) : fullName.substr(sPos, nameExtraStart - sPos);
	// strip trailing numbers applied to duplicate object names in the newest version of assimp
	while (isdigit(name[name.length() - 1]))
		name = name.substr(0, name.length() - 1);
	
	// strip constructor arguments
	if (name[name.length() - 1] == ')') 
		name = name.substr(0,name.find("("));

	return name;
}

/*
extract all arguments from the specified node name in string form
@param name: the node name from which to extract the arguments
@returns: a vector of extracted string arguments
*/
std::vector<std::string> extractNameArgs(std::string name) {
	std::vector<std::string> args;
	// find the start and end of the argument list
	std::size_t sParenPos = name.find("(");
	std::size_t eParenPos = name.find(")");
	if (sParenPos == std::string::npos || eParenPos == std::string::npos)
		return args;

	// sequentually extract arguments using string.find
	size_t last = 0; 
	std::string delimiter = ",";
	std::string s = name.substr(sParenPos+1,eParenPos-sParenPos-1);
	for (size_t next = 0; next = s.find(delimiter, last) != std::string::npos; last = next+1)
		args.emplace_back(s.substr(last, next - last));
	args.emplace_back(s.substr(last));

	return args;
}


// TODO: do something more elegant than using a global temp struct
/*
process the current node while loading a map, either extracting a single piece of transform data or finalizing the current object / static mesh
@param node: the node we are currently processing
@param scene: the overall scene returned by ASSIMP when loading the initial map model
@param directory: the directory in which the map resides
*/
void processMapNode(aiNode *node, const aiScene *scene, std::string directory) {
	// determine the full name and real name of the current node
	tempProp.fullName = node->mName.C_Str();
	std::string name = stripNodeName(tempProp.fullName);

	// extract the transform data from the current node
	aiVector3D aiPos, aiRot, aiScale;
	node->mTransformation.Decompose(aiScale, aiRot, aiPos);
	glm::vec3 pos = glm::vec3(aiPos.x, aiPos.y, aiPos.z);
	glm::vec3 rot = glm::vec3(aiRot.x - glm::half_pi<float>(), -aiRot.y, aiRot.z);
	glm::vec3 scale = glm::vec3(aiScale.x, aiScale.y, aiScale.z);

	// check what type of data the current node is designated to store, and update the corresponding transform data if relevant
	bool isTransformNode = false;
	if (tempProp.fullName.find("$_Translation") != std::string::npos) {
		tempProp.pos = pos;
		isTransformNode = true;
	}
	else if (tempProp.fullName.find("$_PreRotation") != std::string::npos) {
		// note: pre-rotation data is ignored and manually applied to rotation above
		isTransformNode = true;
	}
	else if (tempProp.fullName.find("$_GeometricTranslation") != std::string::npos) {
		isTransformNode = true;
		tempProp.geoPos = pos;
	}
	else if (tempProp.fullName.find("$_Rotation") != std::string::npos) {
		tempProp.rot = rot;
		isTransformNode = true;
	}
	else if (tempProp.fullName.find("$_Scaling") != std::string::npos) {
		tempProp.scale = scale;
		isTransformNode = true;
	}

	if (!isTransformNode) {
		std::vector<std::string> argList = extractNameArgs(tempProp.fullName);
		// convert nodes starting with o_ into GameObject instances using the named model
		if (strncmp(tempProp.fullName.c_str(), "o_", 2) == 0) {
			// load a barebones physics enabled model
			//std::cout << "generating object: " << name << std::endl;
			gameObjects.emplace_back(new GameObject(tempProp.pos + tempProp.geoPos, tempProp.rot, tempProp.scale, name));
			goto clearTransform;
		}
		else if (strncmp(tempProp.fullName.c_str(), "go_", 3) == 0) {
			// load a class
			//std::cout << "generating instance of GameObject: " << name << std::endl;
			instantiateGameObject(name, tempProp.pos + tempProp.geoPos, tempProp.rot, tempProp.scale, argList);
			goto clearTransform;
		}
		else if (strncmp(tempProp.fullName.c_str(), "l_", 2) == 0) {
			//std::cout << "generating light: " << name << std::endl;
			// create a light
			instantiateLight(name,tempProp.pos + tempProp.geoPos, tempProp.rot, tempProp.scale, argList);
			goto clearTransform;
		}
		else if (node->mNumMeshes > 0) {
			// once we've reached the final node for a static mesh (non-object) process the mesh data and store it as a new model in the scene
			// generate a new model from the mesh list
			//TODO: consider using name here rather than fullName so we can re-use static geometry too
			//std::cout << "generating static geometry: " << tempProp.fullName << std::endl;
			std::shared_ptr<Model> baseModel = std::make_shared<Model>();
			for (unsigned int i = 0; i < node->mNumMeshes; ++i)
				baseModel->meshes.push_back(baseModel->processMesh(scene->mMeshes[node->mMeshes[i]], scene, directory));
			baseModel->calculateCollisionShape();
			models.insert({ tempProp.fullName, baseModel });
			gameObjects.emplace_back(new GameObject(tempProp.pos + tempProp.geoPos, glm::vec3(tempProp.rot.x , tempProp.rot.y, tempProp.rot.z), tempProp.scale, tempProp.fullName,false,true,false));
			goto clearTransform;
		}
		else {
		clearTransform:
			// reset the accumulated transform properties first thing once we finish building an object
			tempProp.pos = glm::vec3(0, 0, 0);
			tempProp.geoPos = glm::vec3(0, 0, 0);
			tempProp.rot = glm::vec3(-glm::half_pi<float>(), 0, 0);
			tempProp.scale = glm::vec3(1, 1, 1);
		}
	}

	// recurse over child nodes regardless of current node type
	for (unsigned int i = 0; i < node->mNumChildren; ++i)
		processMapNode(node->mChildren[i], scene, directory);
}
/*
load the specified map, instantiating all referenced objects and creating an empty object to house the static geometry
@param mapName: the name of the map to load
*/
void loadMap(std::string mapName) {
	// TODO: don't use hard-coded map folder
	// load the map as a typical model via ASSIMP
	std::string directory = FileSystem::getPath("maps/" + mapName + ".fbx");
	std::string path = directory.substr(0, directory.find_last_of('/'));
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(directory, aiMapProcessFlags);
	// check for errors
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		ERROR(std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl);
		return;
	}
	std::cout << "Loading map '" << mapName << "'" << std::endl;
	// now process nodes recursively with custom instructions since this is a map model
	tempProp.prevName = "";
	processMapNode(scene->mRootNode, scene, path);
	SUCCESS(std::cout << "Finished loading map '" << mapName << "'" << std::endl);
}

struct Character {
	GLuint     TextureID;  // ID handle of the glyph texture
	glm::ivec2 Size;       // Size of glyph
	glm::ivec2 Bearing;    // Offset from baseline to left/top of glyph
	GLuint     Advance;    // Offset to advance to next glyph
};

std::unordered_map<std::string,std::unordered_map<int,Character[numFontCharacters]>> fonts;

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
*/
void renderText(std::string fontName, int fontSize, Shader &s, std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color) {
	if (!fonts[fontName].count(fontSize)) {
		ERROR(std::cout << "Error: font '" << fontName << "' at size '" << fontSize << "' not found in fonts map; please load this (font,size) pair and try again" << std::endl);
		return;
	}
	// Activate corresponding render state	
	s.use();
	glUniform3f(glGetUniformLocation(s.ID, "textColor"), color.x, color.y, color.z);
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(textVAO);

	// Iterate through all characters
	std::string::const_iterator c;
	for (c = text.begin(); c != text.end(); ++c) {
		Character ch = fonts[fontName][fontSize][*c];

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
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "CPPGameEngine", fullScreen ? glfwGetPrimaryMonitor() : NULL,NULL);
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
initialize bullet physics
*/
void initBullet() {
	// collision configuration contains default setup for memory, collision setup. Advanced users can create their own configuration.
	bulletData.collisionConfiguration = new btDefaultCollisionConfiguration();

	// use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
	bulletData.dispatcher = new btCollisionDispatcher(bulletData.collisionConfiguration);

	// btDbvtBroadphase is a good general purpose broadphase. You can also try out btAxis3Sweep.
	bulletData.overlappingPairCache = new btDbvtBroadphase();

	// the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
	bulletData.solver = new btSequentialImpulseConstraintSolver;

	bulletData.dynamicsWorld = new btDiscreteDynamicsWorld(bulletData.dispatcher, bulletData.overlappingPairCache, bulletData.solver, bulletData.collisionConfiguration);

	bulletData.dynamicsWorld->setGravity(btVector3(0, -10, 0));
}

/*
cleanup the data allocated by bullet physics
*/
void cleanupBullet() {
	// remove the rigidbodies from the dynamics world
	for (int i = bulletData.dynamicsWorld->getNumCollisionObjects() - 1; i >= 0; --i) {
		btCollisionObject* obj = bulletData.dynamicsWorld->getCollisionObjectArray()[i];
		bulletData.dynamicsWorld->removeCollisionObject(obj);
	}

	// remove collision shapes
	bulletData.collisionShapes.clear();

	// delete dynamics world
	delete bulletData.dynamicsWorld;

	// delete solver
	delete bulletData.solver;

	// delete broadphase
	delete bulletData.overlappingPairCache;

	// delete dispatcher
	delete bulletData.dispatcher;

	delete bulletData.collisionConfiguration;
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

// Helper class; draws the world as seen by Bullet.
// This is very handy to see it Bullet's world matches yours
// How to use this class :
// Declare an instance of the class :
// 
// dynamicsWorld->setDebugDrawer(&mydebugdrawer);
// Each frame, call it :
// mydebugdrawer.SetMatrices(ViewMatrix, ProjectionMatrix);
// dynamicsWorld->debugDrawWorld();
class BulletDebugDrawer_OpenGL : public btIDebugDraw {
public:
	void SetMatrices(Shader s, glm::mat4 pViewMatrix, glm::mat4 pProjectionMatrix)
	{
		glUniformMatrix4fv(glGetUniformLocation(s.ID, "projection"), 1, GL_FALSE, glm::value_ptr(pProjectionMatrix));
		glUniformMatrix4fv(glGetUniformLocation(s.ID, "view"), 1, GL_FALSE, glm::value_ptr(pViewMatrix));
	}

	virtual void drawLine(const btVector3& from, const btVector3& to, const btVector3& color)
	{
		// Vertex data
		GLfloat points[12];

		points[0] = from.x();
		points[1] = from.y();
		points[2] = from.z();
		points[3] = color.x();
		points[4] = color.y();
		points[5] = color.z();

		points[6] = to.x();
		points[7] = to.y();
		points[8] = to.z();
		points[9] = color.x();
		points[10] = color.y();
		points[11] = color.z();

		glDeleteBuffers(1, &VBO);
		glDeleteVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenVertexArrays(1, &VAO);
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
	virtual void drawContactPoint(const btVector3 &, const btVector3 &, btScalar, int, const btVector3 &) {}
	virtual void reportErrorWarning(const char *) {}
	virtual void draw3dText(const btVector3 &, const char *) {}
	virtual void setDebugMode(int p) {
		m = p;
	}
	int getDebugMode(void) const { return m; }
	int m;
};

/*
cast a ray from the specified NDC coordinates using the given proj/view matrices, returning the first hit object
@param projection: the projection matrix to cast frrom
@param view: the view ematrix to cast from
@param x: the x coordinate of the raycast (in NDC space)
@param y: the y coordinate of the raycast (in NDC space)
@returns: a pointer to the hit collision object (from whom you can get more useful info via getUserPointer) or NULL if no object was hit
*/
std::unique_ptr<btCollisionWorld::ClosestRayResultCallback> rayCast(glm::mat4 projection, glm::mat4 view, float x=0, float y=0) {
	// object picking
	// The ray Start and End positions, in Normalized Device Coordinates (Have you read Tutorial 4 ?)
	glm::vec4 lRayStart_NDC(x,y, -1.0, 1.0f);
	glm::vec4 lRayEnd_NDC(x, y, 0.0, 1.0f);

	// inverse transform matrices to camera space
	glm::mat4 M = glm::inverse(projection*view);
	glm::vec4 lRayStart_world = M * lRayStart_NDC; lRayStart_world /= lRayStart_world.w;
	glm::vec4 lRayEnd_world = M * lRayEnd_NDC; lRayEnd_world /= lRayEnd_world.w;

	// get ray direction
	glm::vec3 lRayDir_world(lRayEnd_world - lRayStart_world);
	lRayDir_world = glm::normalize(lRayDir_world);
	glm::vec3 out_origin = glm::vec3(lRayStart_world);
	glm::vec3 out_direction = glm::normalize(lRayDir_world);

	// ray test
	glm::vec3 out_end = out_origin + out_direction*1000.0f;

	std::unique_ptr<btCollisionWorld::ClosestRayResultCallback> RayCallback(new btCollisionWorld::ClosestRayResultCallback(
		btVector3(out_origin.x, out_origin.y, out_origin.z),
		btVector3(out_end.x, out_end.y, out_end.z)
	));
	bulletData.dynamicsWorld->rayTest(
		btVector3(out_origin.x, out_origin.y, out_origin.z),
		btVector3(out_end.x, out_end.y, out_end.z),
		*RayCallback
	);
	return RayCallback;
}

/*
reset all input events that occur for a single frame only
*/
void resetSingleFrameInput() {
	// reset mouse events
	mousePressedLeft = false;
	mouseReleasedLeft = false;
	mousePressedRight = false;
	mouseReleasedRight = false;
}

int main() {
	// note: uncomment me and set me to the proper directory if you need to run Dr. Memory
	// _chdir("C:\\Users\\Ryan\\Documents\\git-projects\\CPPGameEngine\\CPPGameEngine");
	GLFWwindow* window = initWindow();
	initGBuffer();
	initBullet();
	initFreetype();
	freetypeLoadFont("Inter-Regular", 24);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
	glEnable(GL_CULL_FACE);

	// configure depth map FBO
	// -----------------------
	const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
#define NR_LIGHTS 4
	unsigned int depthMapFBO[NR_LIGHTS];
	unsigned int depthCubemap[NR_LIGHTS];
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

	// build and compile shaders
	// -------------------------
	Shader shaderGeometryPass("g_buffer.vs", "g_buffer.fs");
	Shader shaderLightingPass("deferred_shading.vs", "deferred_shading.fs");
	Shader shaderLightBox("deferred_light_box.vs", "deferred_light_box.fs");
	Shader debugLineShader("debugLineShader.vs", "debugLineShader.fs");
	Shader textShader("textShader.vs", "textShader.fs");
	glm::mat4 orthoProjection = glm::ortho(0.0f, static_cast<GLfloat>(SCR_WIDTH), 0.0f, static_cast<GLfloat>(SCR_HEIGHT));
	Shader pointShadowsDepth("point_shadows_depth.vs", "point_shadows_depth.fs", "point_shadows_depth.gs");

	// load models
	// -----------
	Model::defaultDiffuseMap.id = textureFromFile("defaultDiffuseMap.png", ".");
	Model::defaultDiffuseMap.type = "texture_diffuse";
	Model::defaultDiffuseMap.path = "defaultDiffuseMap.png";

	Model::defaultNormalMap.id = textureFromFile("defaultNormalMap.png", ".");
	Model::defaultNormalMap.type = "texture_normal";
	Model::defaultNormalMap.path = "defaultNormalMap.png";

	Model::defaultSpecularMap.id = textureFromFile("defaultSpecularMap.png", ".");
	Model::defaultSpecularMap.type = "texture_specular";
	Model::defaultSpecularMap.path = "defaultSpecularMap.png";

	Model::defaultHeightMap.id = textureFromFile("defaultHeightMap.png", ".");
	Model::defaultHeightMap.type = "texture_height";
	Model::defaultHeightMap.path = "defaultHeightMap.png";
	
	// load map
	loadMap("hallway");
	// enable anisotropic filtering if supported
	if (glfwExtensionSupported("GL_EXT_texture_filter_anisotropic"))
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &anisoFilterAmount);
	if (anisoFilterAmount > 0) {
		for (std::pair<const std::string, Texture> kv : Model::texturesLoaded) {
			glBindTexture(GL_TEXTURE_2D, kv.second.id);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisoFilterAmount);
		}
		glBindTexture(GL_TEXTURE_2D, 0);
	};

	// shader configuration
	// --------------------
	shaderLightingPass.use();
	shaderLightingPass.setInt("gPosition", 0);
	shaderLightingPass.setInt("gNormal", 1);
	shaderLightingPass.setInt("gAlbedoSpec", 2);
	shaderLightingPass.setInt("depthMap0", 3);
	shaderLightingPass.setInt("depthMap1", 4);
	shaderLightingPass.setInt("depthMap2", 5);
	shaderLightingPass.setInt("depthMap3", 6);

	BulletDebugDrawer_OpenGL * debugDrawer = new BulletDebugDrawer_OpenGL();
	debugDrawer->setDebugMode(btIDebugDraw::DBG_DrawConstraints);
	bulletData.dynamicsWorld->setDebugDrawer(debugDrawer);

	btGeneric6DofConstraint* holdConstraint = NULL;
	btRigidBody* holdBody = NULL;
	btVector3 btRayTo;
	btVector3 btRayFrom;
	btScalar m_pickDist;
	float maxPickDist = 2.5f;
	// render loop
	// -----------
	while (!glfwWindowShouldClose(window)) {
		// update frame
		updateTime();
		resetSingleFrameInput();
		glfwPollEvents();
		processInput(window);

		// update physics
		// TODO: don't hardcode 60fps physics
		//bulletData.dynamicsWorld->stepSimulation(1.f / 60.f, 10);
		bulletData.dynamicsWorld->stepSimulation(deltaTime, 10, 1. / 240.);
		//bulletData.dynamicsWorld->stepSimulation(deltaTime, 10, 1. / 60.);

		// update objects
		for (int i = 0; i < gameObjects.size(); ++i)
			gameObjects[i]->update(deltaTime);
		for (int i = 0; i < lights.size(); ++i)
			lights[i]->update(deltaTime);

		// get updated view / projection matrices
#define near_plane 0.1f
#define far_plane 1000.0f
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, near_plane, far_plane);
		glm::mat4 view = camera.GetViewMatrix();

		std::unique_ptr<btCollisionWorld::ClosestRayResultCallback> hit = rayCast(projection, view);
		btRayTo = hit->m_rayToWorld;
		btRayFrom = hit->m_rayFromWorld;
		// if we click onn an object, attempt to grab it
		if (mousePressedLeft && (holdBody == NULL) && hit->hasHit()) {
			m_pickDist = (hit->m_hitPointWorld - hit->m_rayFromWorld).length();
			if (m_pickDist < maxPickDist) {
				GameObject* hitObj = (GameObject*) hit->m_collisionObject->getUserPointer();
				if (!(hitObj->model->isStaticMesh) && hitObj->grabbable) {
					holdBody = const_cast<btRigidBody*>(btRigidBody::upcast(hit->m_collisionObject));
					btVector3 localPivot = holdBody->getCenterOfMassTransform().inverse() * hit->m_hitPointWorld;
					btTransform tr;
					tr.setIdentity();
					tr.setOrigin(localPivot);
					holdConstraint = new btGeneric6DofConstraint(*holdBody, tr, true);
					holdConstraint->setLinearLowerLimit(btVector3(0, 0, 0));
					holdConstraint->setLinearUpperLimit(btVector3(0, 0, 0));
					holdConstraint->setAngularLowerLimit(btVector3(0, 0, 0));
					holdConstraint->setAngularUpperLimit(btVector3(0, 0, 0));
					bulletData.dynamicsWorld->addConstraint(holdConstraint, true);
					for (int i = 0; i < 6; ++i) {
						// CFM (constraint force mixing): increase this to make the constraint softer
						// ERP (error reduction parameter): increase this to fix a greater proportion of the accumulated error each step
						holdConstraint->setParam(BT_CONSTRAINT_STOP_CFM, 0.8f, i);
						holdConstraint->setParam(BT_CONSTRAINT_STOP_ERP, 0.5f, i);
					}
				}
			}
		}

// clear held body velocity upon letting go
// TODO: set held body to player velocity once a player class is defined, ie. riding an elevator
// TODO: make this a proper method once data encapsulation structure has been decided
#define clearHeldBodyVelocity() { \
	holdBody->setLinearVelocity(btVector3(0, 0, 0)); \
	holdBody->setAngularVelocity(btVector3(0, 0, 0)); \
} \

		// throw held object on right mouse button
		if (mousePressedRight && (holdBody != NULL)) {
			float force = .1f;
			btVector3 dir = (btRayTo - btRayFrom).normalize() * force;
			clearHeldBodyVelocity();
			holdBody->applyCentralImpulse(dir);
			goto letGo;
		}

		// release held object on mouse button release
		if (mouseReleasedLeft && (holdBody != NULL)) {
			clearHeldBodyVelocity();
		letGo:
			bulletData.dynamicsWorld->removeConstraint(holdConstraint);
			delete holdConstraint;
			holdConstraint = NULL;
			holdBody = NULL;
		}

		// update held object
		if (holdConstraint != NULL) {
			//keep it at the same picking distance
			holdBody->activate(true);
			btVector3 dir = (btRayTo - btRayFrom).normalize();
			dir *= m_pickDist;
			holdConstraint->getFrameOffsetA().setOrigin(btRayFrom + dir);
		}

		// render
		// 0. create depth cubemap transformation matrices
		// -----------------------------------------------
		glEnable(GL_BLEND);
		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		pointShadowsDepth.use();
		pointShadowsDepth.setFloat("far_plane", far_plane);
		glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT, near_plane, far_plane);
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

				// 0.5. render scene to depth cubemap
				// --------------------------------
				for (unsigned int i = 0; i < 6; ++i)
					pointShadowsDepth.setMat4("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
				pointShadowsDepth.setVec3("lightPos", lightPos);
				// render scene
				for (unsigned int i = 0; i < gameObjects.size(); ++i) {
					pointShadowsDepth.setMat4("model", glm::scale(glm::translate(glm::mat4(1.0f), gameObjects[i]->position) * gameObjects[i]->rotation, gameObjects[i]->scale));
					gameObjects[i]->model->draw(pointShadowsDepth);
				}
			}
		}
		glDisable(GL_BLEND);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// 1. geometry pass: render scene's geometry/color data into gbuffer
		// -----------------------------------------------------------------
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, gBuffer.buffer);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		shaderGeometryPass.use();
		shaderGeometryPass.setMat4("projection", projection);
		shaderGeometryPass.setMat4("view", view);
		shaderGeometryPass.setVec3("viewPos", camera.Position);
		for (unsigned int i = 0; i < gameObjects.size(); ++i) {
			shaderGeometryPass.setMat4("model", glm::scale(glm::translate(glm::mat4(1.0f), gameObjects[i]->position) * gameObjects[i]->rotation, gameObjects[i]->scale));
			gameObjects[i]->model->draw(shaderGeometryPass);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// 2. lighting pass: calculate lighting by iterating over a screen filled quad pixel-by-pixel using the gbuffer's content.
		// -----------------------------------------------------------------------------------------------------------------------
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		shaderLightingPass.use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gBuffer.position);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gBuffer.normal);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, gBuffer.albedoSpec);
		// send light relevant uniforms
		for (unsigned int i = 0; i < lights.size(); ++i) {
			if (lights[i]->on) {
				shaderLightingPass.setFloat("lights[" + std::to_string(i) + "].On", true);
				shaderLightingPass.setVec3("lights[" + std::to_string(i) + "].Position", lights[i]->position);
				shaderLightingPass.setVec3("lights[" + std::to_string(i) + "].Color", lights[i]->color);
				shaderLightingPass.setFloat("lights[" + std::to_string(i) + "].Linear", lights[i]->linear);
				shaderLightingPass.setFloat("lights[" + std::to_string(i) + "].Quadratic", lights[i]->quadratic);
				shaderLightingPass.setFloat("lights[" + std::to_string(i) + "].Radius", lights[i]->radius);
			}
			else {
				shaderLightingPass.setFloat("lights[" + std::to_string(i) + "].On", false);
			}
		}
		shaderLightingPass.setVec3("viewPos", camera.Position);
		// shadow uniforms
		shaderLightingPass.setFloat("far_plane", far_plane);
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

		// 2.5. copy content of geometry's depth buffer to default framebuffer's depth buffer
		// ----------------------------------------------------------------------------------
		glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer.buffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // TODO: internal format of FBO and default framebuffer must match (implementation defined?)
		glBlitFramebuffer(0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0, SCR_WIDTH, SCR_HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// 3. render lights on top of scene
		// --------------------------------
		shaderLightBox.use();
		shaderLightBox.setMat4("projection", projection);
		shaderLightBox.setMat4("view", view);
		for (unsigned int i = 0; i < lights.size(); i++) {
			shaderLightBox.setMat4("model", glm::scale(glm::translate(glm::mat4(1.0f), lights[i]->position), glm::vec3(.1f)));
			shaderLightBox.setVec3("lightColor", lights[i]->on ? lights[i]->color : lights[i]->offColor);
			renderCube();
		}

		// 4. render UI
		// centered point to indicate mouse position for precise object grabbing / interaction
		// TODO: stick me in a "render point" method
		glDisable(GL_DEPTH_TEST);
		debugLineShader.use();
		debugDrawer->SetMatrices(debugLineShader, view, projection);
		GLfloat points[6];
		points[0] = btRayTo.getX();
		points[1] = btRayTo.getY();
		points[2] = btRayTo.getZ();
		points[3] = 0;
		points[4] = 0;
		points[5] = 255;

		glDeleteBuffers(1, &VBO);
		glDeleteVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(points), &points, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), 0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
		glBindVertexArray(0);

		glBindVertexArray(VAO);
		glDrawArrays(GL_POINTS, 0, 1);
		glBindVertexArray(0);

		// debug render bullet data
		bulletData.dynamicsWorld->debugDrawWorld();

		// 5. render text
		textShader.use();
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glUniformMatrix4fv(glGetUniformLocation(textShader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(orthoProjection));
		renderText("Inter-Regular", 24, textShader, "fps: " + std::to_string((int)round(1 / (deltaTime == 0 ? 1 : deltaTime))), 6, 6, 1.0f, glm::vec3(0.5, 0.8f, 0.2f));
		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		
		glfwSwapBuffers(window);
	}
	cleanupBullet();
	glfwTerminate();
	// delete object and model data
	gameObjects.clear();
	models.clear();
}