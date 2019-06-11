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
#include <dNewton.h>
#include <dNewtonCollision.h>
#include <dNewtonDynamicBody.h>
#include <dNewtonPlayerManager.h>

class MyDynamicBody : public dNewtonDynamicBody
{
public:
	MyDynamicBody(dNewton* const world, dFloat mass, const dNewtonCollision* const collision, void* const userData, const dMatrix& matrix)
		:dNewtonDynamicBody(world, mass, collision, userData, &matrix[0][0], NULL)
	{
	}

	// the end application need to overload this function from dNetwonBody
	void OnBodyTransform(const dFloat* const matrix, int threadIndex)
	{
		Update(matrix);
	}

	// the end application need to overload this function from dNetwonDynamicBody
	void OnForceAndTorque(dFloat timestep, int threadIndex)
	{
		// apply gravity force to the body
		dFloat mass;
		dFloat Ixx;
		dFloat Iyy;
		dFloat Izz;

		GetMassAndInertia(mass, Ixx, Iyy, Izz);
		dVector gravityForce(0.0f, -9.8f * mass, 0.0f);
		SetForce(&gravityForce[0]);
	}
};

class GameObject {
public:
	glm::vec3 position;
	glm::mat4 rotation;
	glm::vec3 scale;
	std::shared_ptr<Model> model;
	bool grabbable;
	std::string modelName;
	MyDynamicBody* body;
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
		mass = model->isStaticMesh ? 0.0f : model->volume*averageScale;

		body = new MyDynamicBody(&world, mass, model->collisionShape, NULL, dGetIdentityMatrix());
		
		//NewtonBodySetMassProperties(body, mass, model->collisionShape);

		//TODO: pos, rotation, scale
		/*btQuaternion quat;
		glm::vec3 rotationEA = glm::eulerAngles(rot);
		quat.setEulerZYX(rotationEA.z, rotationEA.y, rotationEA.x); //or quat.setEulerZYX depending on the ordering you want
		startTransform.setRotation(quat);
		*/
	}
	
	/*
	update the GameObject instance
	@param deltaTime: the elapsed time (in seconds) since the previous frame
	*/
	virtual void update(float deltaTime) {
		// update transform position to bullet transform position
		/*NewtonBodyGetPosition(body,)
		position.x = float(trans.getOrigin().getX());
		position.y = float(trans.getOrigin().getY());
		position.z = float(trans.getOrigin().getZ());
		float z, y, x;
		trans.getRotation().getEulerZYX(z, y, x);
		setRotation(glm::vec3(x, y, z));*/
		
	}

	/*
	update the GameObject's rotation from a vec3 of euler angles
	@param rotationEA: the desired rotation (in euler angles) to set
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
#endif
