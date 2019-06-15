#include "stdafx.h"
// engine includes (import order matters here, at least for the time being)
#include "terminalColors.hpp"
#include "filesystem.hpp"
#include "physics.hpp"
#include "settings.hpp"
#include "mapLoader.hpp"
#include "graphics.hpp"
#include "shader.hpp"

#include "mousePicking.hpp"
#include "model.hpp"
std::string displayString = "";
std::unordered_map<std::string, std::string> objectInfoDisplays = { { "cog" , "A rusty old cog. Should still be able to function." } };
#include "GameObject.hpp"
#include "Light.hpp"
#include "timing.hpp"

/*
display an information box detailing the specified object
@param go: the GameObject about which we wish to show information
*/
void displayObjectInfo(GameObject* go) {
	displayString = go->getDisplayString();
	player.camera.controllable = displayString.length() == 0;
}


int main() {
	// note: uncomment me and set me to the proper directory if you need to run Dr. Memory
	// _chdir("C:\\Users\\Ryan\\Documents\\git-projects\\CPPGameEngine\\CPPGameEngine");
	GLFWwindow* window = initWindow();
	initGBuffer();
	initPhysics();
	player.init();
	initFreetype();
	initBuffers();
	freetypeLoadFont("Inter-Regular", 24);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
	glEnable(GL_CULL_FACE);
	glEnable(GL_PROGRAM_POINT_SIZE);

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
	Shader shaderGeometryPass("shaders/g_buffer.vs", "shaders/g_buffer.fs");
	Shader shaderLightingPass("shaders/deferred_shading.vs", "shaders/deferred_shading.fs");
	Shader shaderLightBox("shaders/deferred_light_box.vs", "shaders/deferred_light_box.fs");
	Shader debugLineShader("shaders/debugLineShader.vs", "shaders/debugLineShader.fs");
	Shader textShader("shaders/textShader.vs", "shaders/textShader.fs");
	glm::mat4 orthoProjection = glm::ortho(0.0f, static_cast<GLfloat>(SCR_WIDTH), 0.0f, static_cast<GLfloat>(SCR_HEIGHT));
	Shader pointShadowsDepth("shaders/point_shadows_depth.vs", "shaders/point_shadows_depth.fs", "shaders/point_shadows_depth.gs");

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
	loadMap("bookshelf");
	// enable anisotropic filtering if supported
	applyAnisotropicFiltering();

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

	float maxPickDist = 2.5f;
	// render loop
	// -----------
	bool f3Pressed = false;
	bool debugDraw = false;
	while (!glfwWindowShouldClose(window)) {
		// debug key update
		if (glfwGetKey(window, GLFW_KEY_F3) == GLFW_PRESS)
			f3Pressed = true;
		else if (f3Pressed) {
			f3Pressed = false;
			debugDraw = !debugDraw;
		}
			
		// update frame
		updateTime();
		resetSingleFrameInput();
		glfwPollEvents();

		// update physics
		NewtonUpdate(world, deltaTime);

		// update player
		player.update(window, deltaTime);
		// update objects
		for (int i = 0; i < gameObjects.size(); ++i)
			gameObjects[i]->update(deltaTime);
		for (int i = 0; i < lights.size(); ++i)
			lights[i]->update(deltaTime);

		// picking
		UpdatePickBody(deltaTime);

		if (displayString != "") {
			// clear display string on right mouse button press
			if (mousePressedRight) {
				displayString = "";
				player.camera.controllable = true;
			}
		}

		// render
		// 0. create depth cubemap transformation matrices
		// -----------------------------------------------
		glEnable(GL_BLEND);
		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		pointShadowsDepth.use();
		pointShadowsDepth.setFloat("far_plane", player.camera.far_plane);
		glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT, player.camera.near_plane, player.camera.far_plane);
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
		shaderGeometryPass.setMat4("projection", player.camera.projection);
		shaderGeometryPass.setMat4("view", player.camera.view);
		shaderGeometryPass.setVec3("viewPos", player.camera.Position);
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
		shaderLightingPass.setVec3("viewPos", player.camera.Position);
		// shadow uniforms
		shaderLightingPass.setFloat("far_plane", player.camera.far_plane);
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
		shaderLightBox.setMat4("projection", player.camera.projection);
		shaderLightBox.setMat4("view", player.camera.view);
		for (unsigned int i = 0; i < lights.size(); i++) {
			shaderLightBox.setMat4("model", glm::scale(glm::translate(glm::mat4(1.0f), lights[i]->position), glm::vec3(.1f)));
			shaderLightBox.setVec3("lightColor", lights[i]->on ? lights[i]->color : lights[i]->offColor);
			renderCube();
		}

		// 4. render UI
		// centered point to indicate mouse position for precise object grabbing / interaction, when nothing is currently being held or observed
		glDisable(GL_DEPTH_TEST);
		debugLineShader.use();
		glUniformMatrix4fv(glGetUniformLocation(debugLineShader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(player.camera.projection));
		glUniformMatrix4fv(glGetUniformLocation(debugLineShader.ID, "view"), 1, GL_FALSE, glm::value_ptr(player.camera.view));

		if (debugDraw) {
			debugDrawNewton();
		}

		if (displayString == "") {
			// convert center position into camera coordinates
			glm::mat4 M = glm::inverse(player.camera.projection*player.camera.view);
			glm::vec4 lRayStart_world = M * glm::vec4(0,0, 0,1); lRayStart_world /= lRayStart_world.w;
			debugDrawPoint(glm::vec3(lRayStart_world.x, lRayStart_world.y, lRayStart_world.z), glm::vec3(255, 255, 255));
		}

		// 5. render text
		textShader.use();
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glUniformMatrix4fv(glGetUniformLocation(textShader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(orthoProjection));
		renderText("Inter-Regular", 24, textShader, "fps: " + std::to_string((int)round(1 / (deltaTime == 0 ? 1 : deltaTime))), 6, 6, 1.0f, glm::vec3(0.5, 0.8f, 0.2f));
		// show display string, if active
		if (displayString != "") {
			renderText("Inter-Regular", 24, textShader, displayString, SCR_WIDTH / 2, SCR_HEIGHT / 2, 1.0f, glm::vec3(1, 1, 1), true);
		}
		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		
		glfwSwapBuffers(window);

		//system("pause");
		//break;
	}
	cleanupPhysics();
	glfwTerminate();
	// delete object and model data
	gameObjects.clear();
	models.clear();
}