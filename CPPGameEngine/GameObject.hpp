#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <glad/glad.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "mesh.hpp"
#include "shader.hpp"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <memory>
#include <glm/gtx/quaternion.hpp>
#include <dVector.h>
#include <dMatrix.h>
#include <Newton.h>
#include <dNewton.h>
#include <dNewtonCollision.h>
#include <dNewtonDynamicBody.h>
#include <dNewtonPlayerManager.h>

void cb_applyForce(const NewtonBody* const body, dFloat timestep, int threadIndex);
class GameObject {
public:
	glm::vec3 position;
	glm::mat4 rotation;
	glm::vec3 scale;
	std::shared_ptr<Model> model;
	bool grabbable;
	std::string modelName;
	NewtonBody* body;
	dFloat mass;

	/*
	GameObject constructor: creates a new GameObject with the specified transforms and model
	@param position: the initial tranlation of this GameObject
	@param rotationEA: the inital rotation (in Euler Angles) of this GameObject
	@param scale: the initial scale of this GameObject
	@param modelName: the name of the model that this object uses; a reference to the model will be extracted from models, and the model will be hot loaded if not found
	@param makeStatic: whether or not to force the newly created mesh to be static. Note that this has no effect if the mesh has already been created.
	@param grabbable: whether or not the GameObject can be grabbed by the player via object picking
	@param fixInitialRotation: whether or not the initial rotation needs to be fixed (this should be done for instantiated models, not static mesh data baked into a map)
	*/
	GameObject(glm::vec3 position, glm::vec3 rotationEA, glm::vec3 scale, std::string modelName, bool makeStatic = false, bool grabbable = true, bool fixInitialRotation=true) : position(position), scale(scale), grabbable(grabbable), modelName(modelName) {
		setModel(modelName, makeStatic);
		addPhysics(setRotation(rotationEA, fixInitialRotation));
	}

	/*
	return a string detailing information about this object, to be shown when the user right clicks the object
	*/
	virtual std::string getDisplayString() {
		return objectInfoDisplays[modelName];
	}

	/*
	set this GameObject's model to the specified name, creating a new entry in the model dictionary if the name is not already present
	@param modelName: the name of the model to use
	@param makeStatic: whether or not to make the model static, if we create the model
	*/
	void setModel(std::string modelName, bool makeStatic = false) {
		// note: this function should only be called once at initialization, as the object's physics depend on its set model
		std::unordered_map<std::string, std::shared_ptr<Model>>::iterator search = models.find(modelName);
		if (search != models.end())
			model = search->second;
		else {
			// TODO: don't use hard-coded model folder
			model = std::make_shared<Model>(FileSystem::getPath("models/" + modelName + "/" + modelName + ".fbx"), makeStatic);
			models.insert({ modelName, model });
		}
	}
	/*
	grant physics information to this GameObject (collision shape and rigidbody) and add it to the bullet physics simulation
	@param rot: the quaternion representation of our initial rotation
	*/
	void addPhysics(glm::quat rot) {
		float averageScale = (scale.x + scale.y + scale.z) / 3;
		mass = model->isStaticMesh ? 0.0f : model->volume*averageScale*10;
		
		// rotation
		dMatrix tm = glm::value_ptr(rotation);
		
		// translation
		tm.m_posit.m_x += position.x;
		tm.m_posit.m_y += position.y;
		tm.m_posit.m_z += position.z;

		body = NewtonCreateDynamicBody(world, model->collisionShape, &tm[0][0]);
		NewtonBodySetMassMatrix(body, mass, 1, 1, 1);

		// scale
		NewtonBodySetCollisionScale(body, scale.x, scale.y, scale.z);

		// Install the callbacks to track the body positions.
		NewtonBodySetForceAndTorqueCallback(body, cb_applyForce);
		// Attach our custom data structure to the bodies.
		NewtonBodySetUserData(body, (void *)this);
	}
	
	/*
	update the GameObject instance
	@param deltaTime: the elapsed time (in seconds) since the previous frame
	*/
	virtual void update(float deltaTime) {
		/*dMatrix matrix;
		body->InterpolateMatrix(1.0f, &matrix[0][0]);
		
		// position
		position.x = matrix.m_posit.m_x;
		position.y = matrix.m_posit.m_y;
		position.z = matrix.m_posit.m_z;
		std::cout << "position: " << position.x << ", " << position.y << ", " << position.z << std::endl;
		
		// rotation
		dVector euler0, euler1;
		matrix.GetEulerAngles(euler0, euler1);
		setRotation(glm::vec3(euler0.m_x, euler0.m_y, euler0.m_z));
		*/
		//model->collisionShape->DebugRender(matrix,);
		
		/*trans.getRotation().getEulerZYX(z, y, x);
		setRotation(glm::vec3(x, y, z));*/
	}

	/*
	update the GameObject's rotation from a vec3 of euler angles
	@param rotationEA: the desired rotation (in radian euler angles) to set
	@param fixInitialRotation: whether or not the initial rotation needs to be fixed (this should be done for instantiated models, not static mesh data baked into a map)
	*/
	glm::quat setRotation(glm::vec3 rotationEA, bool fixInitialRotation = false) {
		glm::quat q = glm::quat(rotationEA);
		// fix dynamic object initial rotation with a 90 degree offset
		if (fixInitialRotation) 
			q = glm::angleAxis(glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f)) * q;
		rotation = glm::toMat4(q);
		return q;
	}
};

void cb_applyForce(const NewtonBody* const body, dFloat timestep, int threadIndex) {
	// Apply force.
	// TODO: mass seems to affect fall speed; fix that
	dFloat force[3] = { 0, -9.8, 0 };
	NewtonBodySetForce(body, force);

	// Fetch user data and body position.
	GameObject* GO = (GameObject*)NewtonBodyGetUserData(body);
	dFloat pos[4];
	NewtonBodyGetPosition(body, pos);

	// update position
	GO->position.x = pos[0];
	GO->position.y = pos[1];
	GO->position.z = pos[2];

	// update rotation
	dMatrix rot;
	NewtonBodyGetRotation(body, &rot[0][0]);	
	GO->rotation = glm::toMat4(glm::quat(rot[0].m_w, rot[0].m_x, rot[0].m_y, rot[0].m_z));

	// Print info to terminal.
	//printf("BodyID=%d, Sleep=%d, %.2f, %.2f, %.2f\n",
	//	mydata->bodyID, NewtonBodyGetSleepState(body), pos[0], pos[1], pos[2]);
}
#endif
