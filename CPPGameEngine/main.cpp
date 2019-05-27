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
#include "filesystem.h"
#include "shader.h"
#include "camera.h"
float anisoFilterAmount = 0.0f;
#include "model.h"

#include <iostream>
#include <unordered_map>
#include <memory>

std::unordered_map<std::string, std::shared_ptr<Model>> models;

struct BulletData {
	btDiscreteDynamicsWorld* dynamicsWorld;
	btAlignedObjectArray<btCollisionShape*> collisionShapes;
	btSequentialImpulseConstraintSolver* solver;
	btBroadphaseInterface* overlappingPairCache;
	btCollisionDispatcher* dispatcher;
	btDefaultCollisionConfiguration* collisionConfiguration;
} bulletData;

#include "GameObject.h"
#include "Light.h"
std::vector<GameObject> gameObjects;
std::vector<Light> lights;

// settings
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 0.0f));
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

#include "inputUtils.h"
#include "renderUtils.h"

/*
update deltaTime based on the amount of time elapsed since the previous frame
*/
void updateTime() {
	float currentFrame = glfwGetTime();
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;
}

struct ProcessObjectProperties {
	glm::vec3 pos, rot, scale;
	std::string fullName;
} tempProp;

struct GBuffer {
	unsigned int buffer, position, normal, albedoSpec;
} gBuffer;


// TODO: assimp seems to split fbx models into sub-nodes for each transform, so we have to hack things a bit to load fbx maps
// TODO: do something more elegant than using a global temp struct
/*
process the current node while loading a map, either extracting a single piece of transform data or finalizing the current object / static mesh
@param node: the node we are currently processing
@param scene: the overall scene returned by ASSIMP when loading the initial map model
@param directory: the directory in which the map resides
*/
void processMapNode(aiNode *node, const aiScene *scene, std::string directory) {
	// TODO: instantiating object base class with specified model rather than instantiating child classes for now
	// extract the transform data from the current node
	tempProp.fullName = node->mName.C_Str();
	aiVector3D aiPos, aiRot, aiScale;
	node->mTransformation.Decompose(aiScale, aiRot, aiPos);
	glm::vec3 pos = glm::vec3(aiPos.x, aiPos.y, aiPos.z);
	glm::vec3 rot = glm::vec3(aiRot.x, aiRot.z, aiRot.y);
	glm::vec3 scale = glm::vec3(aiScale.x, aiScale.y, aiScale.z);

	// check what type of data the current node is designated to store, and update the corresponding transform data if relevant
	bool finalTransformNode = false;
	if (tempProp.fullName.find("$_Translation") != std::string::npos) {
		tempProp.pos = pos;
	}
	else if (tempProp.fullName.find("$_Rotation") != std::string::npos) {
		// TODO: rotation is being applied to wrong objects; rotation nodes don't seem to be getting parsed enough (experiment with 1 / all diamonds in testMapPhysicsB)
		tempProp.rot = rot;
	}
	else if (tempProp.fullName.find("$_Scaling") != std::string::npos) {
		tempProp.scale = scale;
		finalTransformNode = true;
	}

	// once we've reached the final transform node, we can instantiate our object
	if (finalTransformNode) {
		// convert nodes starting with o_ into GameObject instances using the named model
		if (strncmp(tempProp.fullName.c_str(), "o_", 2) == 0) {
			// load an existing model
			std::size_t nameExtraStart = tempProp.fullName.find("_$Assimp");
			std::string name = nameExtraStart == std::string::npos ? tempProp.fullName.substr(2) : tempProp.fullName.substr(2, nameExtraStart - 2);
			// strip trailing numbers applied to duplicate object names in the newest version of assimp
			while (isdigit(name[name.length() - 1])) {
				name = name.substr(0, name.length() - 1);
			}
			std::cout << "generating instance of object: " << name << std::endl;
			gameObjects.push_back(GameObject(tempProp.pos, tempProp.rot, tempProp.scale, name));
		}
		else if (strncmp(tempProp.fullName.c_str(), "l_", 2) == 0) {
			// create a light
			std::cout << "generating light: " << tempProp.fullName << std::endl;
			lights.push_back(Light(tempProp.pos, glm::vec3(1, 1, 1)));
		}
	}

	// once we've reached the final node for a static mesh (non-object) process the mesh data and store it as a new model in the scene
	if (node->mNumMeshes > 0 && !(strncmp(tempProp.fullName.c_str(), "l_", 2) == 0 || strncmp(tempProp.fullName.c_str(), "o_", 2) == 0)) {
		// generate a new model from the mesh list
		std::cout << "generating static geometry: " << tempProp.fullName << std::endl;
		std::shared_ptr<Model> baseModel(new Model());
		for (unsigned int i = 0; i < node->mNumMeshes; ++i)
			baseModel->meshes.push_back(baseModel->processMesh(scene->mMeshes[node->mMeshes[i]], scene, directory));
		models.insert({ tempProp.fullName, baseModel });
		gameObjects.push_back(GameObject(tempProp.pos, glm::vec3(tempProp.rot.x - glm::half_pi<float>(), tempProp.rot.y, tempProp.rot.z), tempProp.scale, tempProp.fullName, true));
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
		std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
		return;
	}

	// now process nodes recursively with custom instructions since this is a map model
	processMapNode(scene->mRootNode, scene, path);
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

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "CPPGameEngine", 0, NULL);
	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		exit(EXIT_FAILURE);
	}
	
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
		std::cout << "Framebuffer not complete!" << std::endl;
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

	bulletData.dynamicsWorld->setGravity(btVector3(0, -2, 0));
}

/*
cleanup the data allocated by bullet physics
*/
void cleanupBullet() {
	// remove the rigidbodies from the dynamics world and delete them
	for (int i = bulletData.dynamicsWorld->getNumCollisionObjects() - 1; i >= 0; --i) {
		btCollisionObject* obj = bulletData.dynamicsWorld->getCollisionObjectArray()[i];
		btRigidBody* body = btRigidBody::upcast(obj);
		if (body && body->getMotionState()) {
			delete body->getMotionState();
		}
		bulletData.dynamicsWorld->removeCollisionObject(obj);
		delete obj;
	}

	// delete collision shapes
	for (int j = 0; j < bulletData.collisionShapes.size(); j++) {
		btCollisionShape* shape = bulletData.collisionShapes[j];
		bulletData.collisionShapes[j] = 0;
		delete shape;
	}

	// delete dynamics world
	delete bulletData.dynamicsWorld;

	// delete solver
	delete bulletData.solver;

	// delete broadphase
	delete bulletData.overlappingPairCache;

	// delete dispatcher
	delete bulletData.dispatcher;

	delete bulletData.collisionConfiguration;

	// next line is optional: it will be cleared by the destructor when the array goes out of scope
	bulletData.collisionShapes.clear();
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
unsigned int VBO, VAO;
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

int main() {
	GLFWwindow* window = initWindow();
	initGBuffer();
	initBullet();
	glEnable(GL_DEPTH_TEST);

	// build and compile shaders
	// -------------------------
	Shader shaderGeometryPass("g_buffer.vs", "g_buffer.fs");
	Shader shaderLightingPass("deferred_shading.vs", "deferred_shading.fs");
	Shader shaderLightBox("deferred_light_box.vs", "deferred_light_box.fs");
	Shader debugLineShader("debugLineShader.vs", "debugLineShader.fs");

	// load models
	// -----------
	Model::defaultNormalMap.id = textureFromFile("defaultNormalMap.png", ".");
	Model::defaultNormalMap.type = "texture_normal";
	Model::defaultNormalMap.path = "defaultNormalMap.png";
	std::cout << "loaded default normal map: 'defaultNormalMap.png'" << std::endl;

	Model::defaultSpecularMap.id = textureFromFile("defaultSpecularMap.png", ".");
	Model::defaultSpecularMap.type = "texture_specular";
	Model::defaultSpecularMap.path = "defaultSpecularMap.png";
	std::cout << "loaded default specular map: 'defaultSpecularMap.png'" << std::endl;

	Model::defaultHeightMap.id = textureFromFile("defaultHeightMap.png", ".");
	Model::defaultHeightMap.type = "texture_height";
	Model::defaultHeightMap.path = "defaultHeightMap.png";
	std::cout << "loaded default height map: 'defaultHeightMap.png'" << std::endl;
	
	// load map
	loadMap("testMapPhysics");

	// enable anisotropic filtering if supported
	if (glfwExtensionSupported("GL_EXT_texture_filter_anisotropic"))
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &anisoFilterAmount);
	if (anisoFilterAmount > 0) {
		for (int i = 0; i < Model::textures_loaded.size(); ++i) {
			glBindTexture(GL_TEXTURE_2D, Model::textures_loaded[i].id);
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

	BulletDebugDrawer_OpenGL * debugDrawer = new BulletDebugDrawer_OpenGL();
	debugDrawer->setDebugMode(btIDebugDraw::DBG_DrawWireframe);
	bulletData.dynamicsWorld->setDebugDrawer(debugDrawer);

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window)) {
		// update frame
		updateTime();
		glfwPollEvents();
		processInput(window);

		// update physics
		// TODO: don't hardcode 60fps physics
		bulletData.dynamicsWorld->stepSimulation(1.f / 60.f, 10);
		
		// update objects
		for (int i = 0; i < gameObjects.size(); ++i)
			gameObjects[i].update();

		// render
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// 1. geometry pass: render scene's geometry/color data into gbuffer
		// -----------------------------------------------------------------
		glBindFramebuffer(GL_FRAMEBUFFER, gBuffer.buffer);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		shaderGeometryPass.use();
		shaderGeometryPass.setMat4("projection", projection);
		shaderGeometryPass.setMat4("view", view);
		shaderGeometryPass.setVec3("viewPos", camera.Position);
		for (unsigned int i = 0; i < gameObjects.size(); ++i) {
			shaderGeometryPass.setMat4("model", glm::scale(glm::translate(glm::mat4(1.0f), gameObjects[i].position) * gameObjects[i].rotation, gameObjects[i].scale));
			gameObjects[i].model->draw(shaderGeometryPass);
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
			shaderLightingPass.setVec3("lights[" + std::to_string(i) + "].Position", lights[i].position);
			shaderLightingPass.setVec3("lights[" + std::to_string(i) + "].Color", lights[i].color);
			shaderLightingPass.setFloat("lights[" + std::to_string(i) + "].Linear", lights[i].linear);
			shaderLightingPass.setFloat("lights[" + std::to_string(i) + "].Quadratic", lights[i].quadratic);
			shaderLightingPass.setFloat("lights[" + std::to_string(i) + "].Radius", lights[i].radius);
		}
		shaderLightingPass.setVec3("viewPos", camera.Position);
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
			shaderLightBox.setMat4("model", glm::scale(glm::translate(glm::mat4(1.0f), lights[i].position), glm::vec3(0.05f)));
			shaderLightBox.setVec3("lightColor", lights[i].color);
			renderCube();
		}

		// debug render bullet data
		/*debugLineShader.use();
		debugDrawer->SetMatrices(debugLineShader, view, projection);
		bulletData.dynamicsWorld->debugDrawWorld();*/

		glfwSwapBuffers(window);
	}
	cleanupBullet();
	glfwTerminate();
}