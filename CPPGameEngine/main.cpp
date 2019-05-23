#include <glad/glad.h>
#include <GLFW/glfw3.h>
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

#include "filesystem.h"
#include "shader.h"
#include "camera.h"
#include "model.h"

#include <iostream>
#include <unordered_map>
#include <memory>

std::unordered_map<std::string, std::shared_ptr<Model>> models;
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
};
ProcessObjectProperties tempProp;

/*
assimp seems to split fbx models into sub-nodes for each transform, so we have to hack things a bit to load fbx maps
// TODO: document me
// TODO: do something more elegant than using a global temp struct
*/
void processMapNode(aiNode *node, const aiScene *scene, std::string directory) {
	// TODO: instantiating object base class with specified model rather than instantiating child classes for now
	tempProp.fullName = node->mName.C_Str();
	aiVector3D aiPos, aiRot, aiScale;
	node->mTransformation.Decompose(aiScale, aiRot, aiPos);
	glm::vec3 pos = glm::vec3(aiPos.x, aiPos.y, aiPos.z);
	// note: collada (.dae) exported with y-up appears to render correctly with the following adjustments applied to its rotation
	glm::vec3 rot = glm::vec3(aiRot.x, aiRot.z, aiRot.y);
	glm::vec3 scale = glm::vec3(aiScale.x, aiScale.y, aiScale.z);

	bool finalTransformNode = false;
	if (tempProp.fullName.find("$_Translation") != std::string::npos) {
		tempProp.pos = pos;
	}
	else if (tempProp.fullName.find("$_Rotation") != std::string::npos)
		tempProp.rot = rot;
	else if (tempProp.fullName.find("$_Scaling") != std::string::npos) {
		tempProp.scale = scale;
		finalTransformNode = true;
	}

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
			std::cout << "position: " << tempProp.pos.x << "," << tempProp.pos.y << "," << tempProp.pos.z << std::endl;
			std::cout << "rotation: " << tempProp.rot.x << "," << tempProp.rot.y << "," << tempProp.rot.z << std::endl;
			std::cout << "scale:    " << tempProp.scale.x << "," << tempProp.scale.y << "," << tempProp.scale.z << std::endl;
		}
		else if (strncmp(tempProp.fullName.c_str(), "l_", 2) == 0) {
			// create a light
			std::cout << "generating light: " << tempProp.fullName << std::endl;
			std::cout << "position: " << tempProp.pos.x << "," << tempProp.pos.y << "," << tempProp.pos.z << std::endl;
			lights.push_back(Light(tempProp.pos, glm::vec3(1, 1, 1)));
		}
	}
	if (node->mNumMeshes > 0 && !(strncmp(tempProp.fullName.c_str(), "l_", 2) == 0 || strncmp(tempProp.fullName.c_str(), "os_", 2) == 0)) {
		// generate a new model from the mesh list
		std::cout << "generating static geometry: " << tempProp.fullName << std::endl;
		std::cout << "position: " << tempProp.pos.x << "," << tempProp.pos.y << "," << tempProp.pos.z << std::endl;
		std::cout << "rotation: " << tempProp.rot.x << "," << tempProp.rot.y << "," << tempProp.rot.z << std::endl;
		std::cout << "scale:    " << tempProp.scale.x << "," << tempProp.scale.y << "," << tempProp.scale.z << std::endl;
		std::shared_ptr<Model> baseModel(new Model());
		for (unsigned int i = 0; i < node->mNumMeshes; ++i)
			baseModel->meshes.push_back(baseModel->processMesh(scene->mMeshes[node->mMeshes[i]], scene, directory));
		models.insert({ tempProp.fullName, baseModel });
		gameObjects.push_back(GameObject(tempProp.pos, glm::vec3(tempProp.rot.x - glm::half_pi<float>(), tempProp.rot.y, tempProp.rot.z), tempProp.scale, tempProp.fullName));
	}
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

int main() {
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
		return -1;
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
		return -1;
	}

	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);

	// build and compile shaders
	// -------------------------
	Shader shaderGeometryPass("g_buffer.vs", "g_buffer.fs");
	Shader shaderLightingPass("deferred_shading.vs", "deferred_shading.fs");
	Shader shaderLightBox("deferred_light_box.vs", "deferred_light_box.fs");

	// load models
	// -----------
	Model::defaultHeightMap.id = textureFromFile("defaultHeightMap.png", ".");
	Model::defaultHeightMap.type = "texture_height";
	Model::defaultHeightMap.path = "defaultHeightMap.png";
	std::cout << "loaded default height map: 'defaultHeightMap.png'" << std::endl;
	
	// load map
	loadMap("testMapB");

	// configure g-buffer framebuffer
	// ------------------------------
	unsigned int gBuffer;
	glGenFramebuffers(1, &gBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
	unsigned int gPosition, gNormal, gAlbedoSpec;
	// position color buffer
	glGenTextures(1, &gPosition);
	glBindTexture(GL_TEXTURE_2D, gPosition);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);
	// normal color buffer
	glGenTextures(1, &gNormal);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);
	// color + specular color buffer
	glGenTextures(1, &gAlbedoSpec);
	glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);
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

	// shader configuration
	// --------------------
	shaderLightingPass.use();
	shaderLightingPass.setInt("gPosition", 0);
	shaderLightingPass.setInt("gNormal", 1);
	shaderLightingPass.setInt("gAlbedoSpec", 2);

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window)) {
		// update frame
		updateTime();
		processInput(window);

		// render
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// 1. geometry pass: render scene's geometry/color data into gbuffer
		// -----------------------------------------------------------------
		glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 model = glm::mat4(1.0f);
		shaderGeometryPass.use();
		shaderGeometryPass.setMat4("projection", projection);
		shaderGeometryPass.setMat4("view", view);
		shaderGeometryPass.setVec3("viewPos", camera.Position);
		for (unsigned int i = 0; i < gameObjects.size(); ++i) {
			model = glm::mat4(1.0f);
			model = glm::translate(model, gameObjects[i].position);
			model *= gameObjects[i].rotation;
			model = glm::scale(model, gameObjects[i].scale);
			shaderGeometryPass.setMat4("model", model);
			(*gameObjects[i].model).draw(shaderGeometryPass);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// 2. lighting pass: calculate lighting by iterating over a screen filled quad pixel-by-pixel using the gbuffer's content.
		// -----------------------------------------------------------------------------------------------------------------------
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		shaderLightingPass.use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gPosition);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gNormal);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
		// send light relevant uniforms
		for (unsigned int i = 0; i < lights.size(); ++i) {
			shaderLightingPass.setVec3("lights[" + std::to_string(i) + "].Position", lights[i].position);
			shaderLightingPass.setVec3("lights[" + std::to_string(i) + "].Color", lights[i].color);
			// update attenuation parameters and calculate radius
			const float constant = 1.0; // note that we don't send this to the shader, we assume it is always 1.0 (in our case)
			const float linear = lights[i].linear;
			const float quadratic = lights[i].quadratic;
			shaderLightingPass.setFloat("lights[" + std::to_string(i) + "].Linear", linear);
			shaderLightingPass.setFloat("lights[" + std::to_string(i) + "].Quadratic", quadratic);
			// then calculate radius of light volume/sphere
			const float maxBrightness = std::fmaxf(std::fmaxf(lights[i].color.r, lights[i].color.g), lights[i].color.b);
			float radius = (-linear + std::sqrt(linear * linear - 4 * quadratic * (constant - (256.0f / 5.0f) * maxBrightness))) / (2.0f * quadratic);
			shaderLightingPass.setFloat("lights[" + std::to_string(i) + "].Radius", radius);
		}
		shaderLightingPass.setVec3("viewPos", camera.Position);
		// finally render quad
		renderQuad();

		// 2.5. copy content of geometry's depth buffer to default framebuffer's depth buffer
		// ----------------------------------------------------------------------------------
		glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // TODO: internal format of FBO and default framebuffer must match (implementation defined?)
		glBlitFramebuffer(0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0, SCR_WIDTH, SCR_HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// 3. render lights on top of scene
		// --------------------------------
		shaderLightBox.use();
		shaderLightBox.setMat4("projection", projection);
		shaderLightBox.setMat4("view", view);
		for (unsigned int i = 0; i < lights.size(); i++) {
			model = glm::mat4(1.0f);
			model = glm::translate(model, lights[i].position);
			model = glm::scale(model, glm::vec3(0.05f));
			shaderLightBox.setMat4("model", model);
			shaderLightBox.setVec3("lightColor", lights[i].color);
			renderCube();
		}


		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}