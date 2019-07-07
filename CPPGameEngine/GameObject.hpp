#pragma once
#include "stdafx.h"
#include "mesh.hpp"
#include "shader.hpp"
#include "model.hpp"
using namespace physx;
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
		// calculate mass and prepare physics data structures
		float averageScale = (scale.x + scale.y + scale.z) / 3;
		mass = model->isStaticMesh ? 0.0f : model->volume*averageScale*10;
		PxQuat physRot(rot.x, rot.y, rot.z, rot.w);
		PxMeshScale physScale(PxVec3(scale.x,scale.y,scale.z), PxQuat(PxIdentity));
		
		// create a static body if our model is static
		if (model->isStaticMesh) {
			body = gPhysics->createRigidStatic(PxTransform(PxVec3(position.x,position.y,position.z), physRot));
			PxRigidActorExt::createExclusiveShape(*body, PxTriangleMeshGeometry((PxTriangleMesh*)model->collisionMesh, physScale), *gMaterial);
			gScene->addActor(*body);
		}
		// create a dynamic body if our model is non-static
		else {
			body = gPhysics->createRigidDynamic(PxTransform(PxVec3(position.x, position.y, position.z),physRot));
			PxRigidActorExt::createExclusiveShape(*body, PxConvexMeshGeometry((PxConvexMesh*)model->collisionMesh, physScale), *gMaterial);
			body->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, false);
			((PxRigidDynamic*)body)->setAngularVelocity(PxVec3(0.f, 0.f, 0.f));
			//body->setAngularDamping(0.f);
			gScene->addActor(*body);
		}
		// store a pointer to this GameObject in the body's data field
		body->userData = this;
	}
	
	/*
	update the GameObject instance
	@param deltaTime: the elapsed time (in seconds) since the previous frame
	*/
	virtual void update(float deltaTime) {
		// update position and rotation to match the physics body
		PxTransform pose = body->getGlobalPose();
		position.x = pose.p.x; position.y = pose.p.y; position.z = pose.p.z;
		rotation = glm::toMat4(glm::quat(pose.q.w, pose.q.x, pose.q.y, pose.q.z));
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

	/*
	apply force prior to the current object prior to the next simulation step
	@param timestep:
	@param threadIjdex
	*/
	virtual void applyForceCallback(float timestep, int threadIndex) {
		// apply gravitational force
		/*dFloat force[3] = { 0, -GRAVITY_STRENGTH * mass * gravityMultiplier * !held, 0 };
		NewtonBodySetForce(body, force);

		// disable omega and torque when held
		if (held) {
			dVector zeroVector(0, 0, 0);
			NewtonBodySetOmega(body, &zeroVector[0]);
			NewtonBodySetTorque(body, &zeroVector[0]);
		}*/
	}
};

/*
apply force callback; called by newton each time the body is about to be simulated
@param body: the body that is about to be simulated
@param timestep: 
@param threadIndex:
*/
/*void applyForceCallbackRedirect(const NewtonBody* const body, dFloat timestep, int threadIndex) {
	// retrieve the corresponding GameObject from the body's user data
	GameObject* GO = (GameObject*)NewtonBodyGetUserData(body);
	// allow the gameObject to handle applying force
	GO->applyForceCallback(timestep, threadIndex);
}*/