#pragma once
#include "physics.hpp"
#include "model.hpp"

class GameObject {
public:
	glm::vec3 position;
	glm::mat4 rotation;
	glm::vec3 scale;
	Model* model;
	bool grabbable;
	std::string modelName;
	PxRigidActor* body;
	float mass;
	float gravityMultiplier = 1;
	bool held = false;
	bool isStatic = false;
	bool usePhysics = true;
	bool drawTwoSided = false;
	bool castShadows = true;

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
	GameObject(glm::vec3 position, glm::vec3 rotationEA, glm::vec3 scale, std::string modelName, int makeStatic = 0, bool grabbable = true, bool fixInitialRotation = true, bool usePhysics = true);

	/*
	return a string detailing information about this object, used in the hallway demo when the user right clicks a GameObject
	*/
	virtual std::string getDisplayString();

	/*
	set this GameObject's model to the specified name, creating a new entry in the model dictionary if the name is not already present
	@param modelName: the name of the model to use
	@param makeStatic: whether or not to make the model static, if we create the model
	*/
	void setModel(std::string modelName, bool makeStatic = false);

	/*
	grant physics information to this GameObject (collision shape and rigidbody) and add it to the bullet physics simulation
	@param rot: the quaternion representation of our initial rotation
	*/
	void addPhysics(glm::quat rot);
	/*
	update the GameObject instance
	*/
	virtual void update();

	/*
	update the GameObject's rotation from a vec3 of euler angles
	@param rotationEA: the desired rotation (in radian euler angles) to set
	@param fixInitialRotation: whether or not the initial rotation needs to be fixed (this should be done for instantiated models, not static mesh data baked into a map)
	*/
	glm::quat setRotation(glm::vec3 rotationEA, bool fixInitialRotation = false);
};