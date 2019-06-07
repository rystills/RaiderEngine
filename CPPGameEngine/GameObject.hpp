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

class GameObject {
public:
	glm::vec3 position;
	glm::mat4 rotation;
	glm::vec3 scale;
	std::shared_ptr<Model> model;
	bool grabbable;

	// bullet data
	std::unique_ptr<btCollisionShape> collisionShape;
	bool useModelCollisionShape = false;  // in some instances we don't need our own collision shape; the model shape suffices. Always use the model shape if this flag is true
	std::unique_ptr<btRigidBody> body;
	std::unique_ptr<btDefaultMotionState> myMotionState;

	/*
	GameObject constructor: creates a new GameObject with the specified transforms and model
	@param position: the initial tranlation of this GameObject
	@param rotationEA: the inital rotation (in Euler Angles) of this GameObject
	@param scale: the initial scale of this GameObject
	@param modelName: the name of the model that this object uses; a reference to the model will be extracted from models, and the model will be hot loaded if not found
	@param makeStatic: whether or not to force the newly created mesh to be static. Note that this has no effect if the mesh has already been created.
	@param grabbable: whether or not the GameObject can be grabbed by the player via object picking
	*/
	GameObject(glm::vec3 position, glm::vec3 rotationEA, glm::vec3 scale, std::string modelName, bool makeStatic = false, bool grabbable = true, bool useSpecialRotationInit=true) : position(position), scale(scale), grabbable(grabbable) {
		//TODO: this should be simplified: the intermediate transformation into a quaternion seems to be overkill
		setModel(modelName, makeStatic);
		addPhysics(useSpecialRotationInit ? setRotationInitial(rotationEA) : setRotation(rotationEA));
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
		// if we don't have any scaling we can just use our mesh's collision shape directly
		if (scale.x == 1 && scale.y == 1 && scale.z == 1)
			useModelCollisionShape = true;
		else {
			// create a scaled container for our mesh's collision shape
			if (model->isStaticMesh)
				// triangle meshes can be shared with non-uniform scaling
				collisionShape = std::make_unique<btScaledBvhTriangleMeshShape>((btBvhTriangleMeshShape*)(model->collisionShape.get()), btVector3(scale.x, scale.y, scale.z));
			else {
				// note: convex hulls can only be shared with uniform scaling, so the scale average will have to be good enough
				collisionShape = std::make_unique<btUniformScalingShape>((btConvexHullShape*)(model->collisionShape.get()), btScalar(averageScale));
				collisionShape->setMargin(model->collisionMargin * averageScale);
			}
			// push back our scaled collision shape
			bulletData.collisionShapes.push_back(collisionShape.get());
		}

		btTransform startTransform;
		startTransform.setIdentity();
		btScalar mass(model->isStaticMesh ? 0.0f : model->volume*averageScale);
		btVector3 localInertia(0, 0, 0);
		if (!model->isStaticMesh)
			(useModelCollisionShape ? model->collisionShape : collisionShape)->calculateLocalInertia(mass, localInertia);
		startTransform.setOrigin(btVector3(position.x, position.y, position.z));
		btQuaternion quat;
		glm::vec3 rotationEA = glm::eulerAngles(rot);
		quat.setEulerZYX(rotationEA.z, rotationEA.y, rotationEA.x); //or quat.setEulerZYX depending on the ordering you want
		startTransform.setRotation(quat);

		// using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
		myMotionState = std::make_unique<btDefaultMotionState>(startTransform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState.get(), (useModelCollisionShape ? model->collisionShape : collisionShape).get(), localInertia);
		body = std::make_unique<btRigidBody>(rbInfo);
		bulletData.dynamicsWorld->addRigidBody(body.get());
		// store our index in the gameObjects vector in userPointer for easy lookup later
		body->setUserPointer((void*)this);
	}
	
	/*
	update the GameObject instance
	@param deltaTime: the elapsed time (in seconds) since the previous frame
	*/
	virtual void update(float deltaTime) {
		// update transform position to bullet transform position
		btTransform trans;
		body->getMotionState()->getWorldTransform(trans);
		position.x = float(trans.getOrigin().getX());
		position.y = float(trans.getOrigin().getY());
		position.z = float(trans.getOrigin().getZ());
		float z, y, x;
		trans.getRotation().getEulerZYX(z, y, x);
		setRotation(glm::vec3(x, y, z));
	}

	/*
	update the GameObject's rotation from a vec3 of euler angles
	@param rotationEA: the desired rotation (in euler angles) to set
	*/
	glm::quat setRotation(glm::vec3 rotationEA) {
		glm::quat q = glm::quat(rotationEA);
		rotation = glm::toMat4(q);
		return q;
	}

	/*
	set the GameObject's rotation from a vec3 of euler angles provided by assimp while loading a 3ds max fbx file
	@param rotationEA: the desired rotation (in euler angles) to set
	@returns: the quaternion calculated prior to conversion to a mat4, for piping into physics initialization
	*/
	glm::quat setRotationInitial(glm::vec3 rotationEA) {
		// for an explanation of how we calculate this quaternion given the rotation application order, see https://gamedev.stackexchange.com/questions/13436/glm-euler-angles-to-quaternion
		float sx = sin(rotationEA.x / 2), sy = sin(rotationEA.y / 2), sz = sin(rotationEA.z / 2);
		float cx = cos(rotationEA.x / 2), cy = cos(rotationEA.y / 2), cz = cos(rotationEA.z / 2);
		glm::quat q = glm::quat(
			cx*cy*cz + sx*sy*sz,  // w
			sx*cy*cz - cx*sy*sz,  // x
			cx*sy*cz + sx*cy*sz,  // y
			cx*cy*sz - sx*sy*cz   // z
		);
		q = glm::angleAxis(glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f)) * q;
		rotation = glm::toMat4(q);
		return q;
	}
};
#endif
