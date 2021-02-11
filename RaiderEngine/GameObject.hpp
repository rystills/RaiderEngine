#pragma once
#include "stdafx.h"
#include "physics.hpp"
#include "model.hpp"

class GameObject {
public:
	glm::vec3 position;
	glm::mat4 rotation;
	glm::vec4 prevRot;
	glm::vec3 scaleVal;
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
	glm::mat4 modelTransform;
	bool isDirty = true;
	inline static GLuint instancedModelVBO;

	/*
	GameObject constructor: creates a new GameObject with the specified transforms and model
	@param position: the initial tranlation of this GameObject
	@param rotationEA: the inital rotation (in Euler Angles) of this GameObject
	@param scale: the initial scale of this GameObject
	@param modelName: the name of the model that this object uses; a reference to the model will be extracted from models, and the model will be hot loaded if not found
	@param makeStatic: static state. 0 = non-static. 1 = static gameObject + static mesh (if the mesh was already loaded in as dynamic, this will be equivalent to 2). 2 = static gameObject + dynamic mesh.
	@param grabbable: whether or not the GameObject can be grabbed by the player via object picking
	@param usePhysics: whether or not this GameObject should be added to the physics world
	*/
	GameObject(glm::vec3 position, glm::vec3 rotationEA, glm::vec3 scale, std::string modelName, int makeStatic = 0, bool grabbable = true, bool usePhysics = true, bool castShadows = true);

	static void initStaticVertexBuffer();

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

	void recalculateModelTransform();

	// transform methods
	// TODO: these methods will have no effect on active physics enabled GameObjects; optional physics overrides should be implemented for each one
	void rotate(const float& angle, glm::vec3 const& axes);
	
	/*
	update the GameObject's rotation from a vec3 of euler angles
	@param rotationEA: the desired rotation (in radian euler angles) to set
	*/
	glm::quat setRotation(glm::vec3 const& rotationEA);

	glm::vec4 GameObject::forwardVec();
	glm::vec4 GameObject::backVec();
	glm::vec4 GameObject::rightVec();
	glm::vec4 GameObject::leftVec();
	glm::vec4 GameObject::upVec();
	glm::vec4 GameObject::downVec();

	void translate(glm::vec3 const& amnt);

	void translate(glm::vec4 const& dir, float const& amnt);

	void setPos(glm::vec3 const& newPos);

	void scale(glm::vec3 const& amnt);

	void setScale(glm::vec3 const& newScale);

	/*
	update the GameObject instance
	*/
	virtual void update();
};