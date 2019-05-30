#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <glad/glad.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "mesh.h"
#include "shader.h"

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

	// bullet data
	std::shared_ptr<btCollisionShape> collisionShape;
	std::shared_ptr<btRigidBody> body;
	int bodyIndex;
	std::shared_ptr<btDefaultMotionState> myMotionState;

	GameObject(glm::vec3 position, glm::vec3 rotationEA, glm::vec3 scale, std::string modelName) : position(position), scale(scale) {
		//TODO: this should be simplified: the intermediate transformation into a quaternion seems to be overkill
		setRotation(rotationEA);
		std::unordered_map<std::string, std::shared_ptr<Model>>::iterator search = models.find(modelName);
		if (search != models.end())
			model = search->second;
		else {
			// TODO: don't use hard-coded model folder
			model = std::make_shared<Model>(FileSystem::getPath("models/" + modelName + "/" + modelName + ".fbx"),false);
			models.insert({ modelName, model });
		}
		addPhysics(rotationEA);
	}

	/*
	grant physics information to this GameObject (collision shape and rigidbody) and add it to the bullet physics simulation
	@param rotationEA: the euler angles rotation vector passed into our constructor
	*/
	void addPhysics(glm::vec3 rotationEA) {
		float averageScale = (scale.x + scale.y + scale.z) / 3;
		// if we don't have any scaling we can just use our mesh's collision shape directly
		if (scale.x == 1 && scale.y == 1 && scale.z == 1)
			collisionShape = model->collisionShape;
		else {
			// create a scaled container for our mesh's collision shape
			if (model->isStaticMesh)
				// triangle meshes can be shared with non-uniform scaling
				collisionShape = std::make_shared<btScaledBvhTriangleMeshShape>((btBvhTriangleMeshShape*)(model->collisionShape.get()), btVector3(scale.x, scale.y, scale.z));
			else {
				// note: convex hulls can only be shared with uniform scaling, so the scale average will have to be good enough
				collisionShape = std::make_shared<btUniformScalingShape>((btConvexHullShape*)(model->collisionShape.get()), btScalar(averageScale));
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
			collisionShape->calculateLocalInertia(mass, localInertia);
		startTransform.setOrigin(btVector3(position.x, position.y, position.z));
		btQuaternion quat;
		quat.setEulerZYX(rotationEA.z, rotationEA.y, rotationEA.x); //or quat.setEulerZYX depending on the ordering you want
		startTransform.setRotation(quat);

		// using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
		myMotionState = std::make_shared<btDefaultMotionState>(startTransform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState.get(), collisionShape.get(), localInertia);
		body = std::make_shared<btRigidBody>(rbInfo);
		bodyIndex = bulletData.dynamicsWorld->getNumCollisionObjects();
		bulletData.dynamicsWorld->addRigidBody(body.get());
		// TODO: set user pointer to this gameObject, set to bodyIndex for now just for testing
		body->setUserPointer((void*)bodyIndex);
	}

	void update() {
		// update transform position to bullet transform position
		btCollisionObject* obj = bulletData.dynamicsWorld->getCollisionObjectArray()[bodyIndex];
		btRigidBody* indBody = btRigidBody::upcast(obj);
		btTransform trans;
		if (indBody && indBody->getMotionState())
			indBody->getMotionState()->getWorldTransform(trans);
		else
			trans = obj->getWorldTransform();
		position.x = float(trans.getOrigin().getX());
		position.y = float(trans.getOrigin().getY());
		position.z = float(trans.getOrigin().getZ());
		float z, y, x;
		trans.getRotation().getEulerZYX(z, y, x);
		setRotation(glm::vec3(x, y, z));
	}

	void setRotation(glm::vec3 rotationEA) {
		rotation = glm::toMat4(glm::quat(rotationEA));
	}
};
#endif
