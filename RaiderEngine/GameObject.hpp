#pragma once
#include "stdafx.h"
#include "mesh.hpp"
#include "shader.hpp"
#include "model.hpp"
extern std::unordered_map<std::string, std::shared_ptr<Model>> models;

class GameObject {
public:
	glm::vec3 position;
	glm::mat4 rotation;
	glm::vec3 scale;
	std::shared_ptr<Model> model;
	bool grabbable;
	std::string modelName;
	PxRigidActor* body;
	float mass;
	float gravityMultiplier = 1;
	bool held = false;
	bool isStatic = false;
	bool usePhysics = true;
	bool drawTwoSided = false;

	/*
	GameObject constructor: creates a new GameObject with the specified transforms and model
	@param position: the initial tranlation of this GameObject
	@param rotationEA: the inital rotation (in Euler Angles) of this GameObject
	@param scale: the initial scale of this GameObject
	@param modelName: the name of the model that this object uses; a reference to the model will be extracted from models, and the model will be hot loaded if not found
	@param makeStatic: static state. 0 = non-static. 1 = static gameObject + static mesh (if the mesh was already loaded in as dynamic, this will be equivalent to 2). 2 = static gameObject + dynamic mesh.
	@param grabbable: whether or not the GameObject can be grabbed by the player via object picking
	@param fixInitialRotation: whether or not the initial rotation needs to be fixed (this should be done for instantiated models, not static mesh data baked into a map)
	@param usePhysics: whether or not this GameObject should be added to the physics world
	*/
	GameObject(glm::vec3 position, glm::vec3 rotationEA, glm::vec3 scale, std::string modelName, int makeStatic = 0, bool grabbable = true, bool fixInitialRotation=true, bool usePhysics = true) : position(position), scale(scale), grabbable(grabbable), modelName(modelName), usePhysics(usePhysics) {
		setModel(modelName, makeStatic == 1);
		isStatic = makeStatic > 0;
		if (isStatic) 
			this->grabbable = false;
		addPhysics(setRotation(rotationEA, fixInitialRotation));
	}

	/*
	return a string detailing information about this object, used in the hallway demo when the user right clicks a GameObject
	*/
	virtual std::string getDisplayString() {
		return "";
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
			model = std::make_shared<Model>(FileSystem::getPath(modelDir + modelName + '/' + modelName + ".fbx"), makeStatic);
			models.insert({ modelName, model });
		}
	}
	/*
	grant physics information to this GameObject (collision shape and rigidbody) and add it to the bullet physics simulation
	@param rot: the quaternion representation of our initial rotation
	*/
	void addPhysics(glm::quat rot) {
		if (!usePhysics) return;
		// calculate mass and prepare physics data structures
		float averageScale = (scale.x + scale.y + scale.z) / 3;
		mass = model->isStaticMesh ? 0.0f : model->volume*averageScale*10;
		PxQuat physRot(rot.x, rot.y, rot.z, rot.w);
		PxVec3 physPot(position.x, position.y, position.z);
		PxMeshScale physScale(PxVec3(scale.x,scale.y,scale.z), PxQuat(PxIdentity));
		
		// our body type depends on our staticness, which may or may not match our model's staticness
		if (isStatic)
			body = gPhysics->createRigidStatic(PxTransform(physPot, physRot));
		else
			body = gPhysics->createRigidDynamic(PxTransform(physPot, physRot));
		// our shape, unlike our body type, depends on our model's staticness
		if (model->isStaticMesh)
			PxRigidActorExt::createExclusiveShape(*body, PxTriangleMeshGeometry((PxTriangleMesh*)model->collisionMesh, physScale), *gMaterial);
		else 
			PxRigidActorExt::createExclusiveShape(*body, PxConvexMeshGeometry((PxConvexMesh*)model->collisionMesh, physScale), *gMaterial);
		// assign default raycast filter
		PxShape* shape;
		body->getShapes(&shape, 1);
		shape->setQueryFilterData(defaultFilterData);
		// store a pointer to this GameObject in the body's data field, then finally add the body to the physics scene
		body->userData = this;
		gScene->addActor(*body);
	}
	
	/*
	update the GameObject instance
	@param deltaTime: the elapsed time (in seconds) since the previous frame
	*/
	virtual void update(float deltaTime) {
		// update position and rotation to match the physics body
		if (usePhysics) {
			PxTransform pose = body->getGlobalPose();
			position.x = pose.p.x; position.y = pose.p.y; position.z = pose.p.z;
			rotation = glm::toMat4(glm::quat(pose.q.w, pose.q.x, pose.q.y, pose.q.z));
		}
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