#include "stdafx.h"
#include "GameObject.hpp"
#include "mesh.hpp"
#include "model.hpp"
#include "settings.hpp"
#include "physics.hpp"

GameObject::GameObject(glm::vec3 position, glm::vec3 rotationEA, glm::vec3 scale, std::string modelName, int makeStatic, bool grabbable, bool fixInitialRotation, bool usePhysics, bool castShadows) :
position(position), scaleVal(scale), grabbable(grabbable), modelName(modelName), usePhysics(usePhysics), castShadows(castShadows) {
	setModel(modelName, makeStatic == 1);
	isStatic = makeStatic > 0;
	if (isStatic)
		this->grabbable = false;
	addPhysics(setRotation(rotationEA, fixInitialRotation));
}

void GameObject::initStaticVertexBuffer() {
	glGenBuffers(1, &instancedModelVBO);
}

std::string GameObject::getDisplayString() {
	return "";
}

void GameObject::setModel(std::string modelName, bool makeStatic) {
	// note: this function should only be called once at initialization, as the object's physics depend on its set model
	if (!models.contains(modelName))
		models.insert({ modelName, std::make_unique<Model>(modelDir + modelName + '/' + modelName + ".fbx", makeStatic) });
	model = models[modelName].get();
}

void GameObject::addPhysics(glm::quat rot) {
	if (!usePhysics) return;
	// calculate mass and prepare physics data structures
	float averageScale = (scaleVal.x + scaleVal.y + scaleVal.z) / 3;
	mass = model->isStaticMesh ? 0.0f : model->volume * averageScale * 1000;
	PxQuat physRot(rot.x, rot.y, rot.z, rot.w);
	PxVec3 physPot(position.x, position.y, position.z);
	PxMeshScale physScale(PxVec3(scaleVal.x, scaleVal.y, scaleVal.z), PxQuat(PxIdentity));

	// our body type depends on our staticness, which may or may not match our model's staticness
	if (isStatic)
		body = gPhysics->createRigidStatic(PxTransform(physPot, physRot));
	else {
		body = gPhysics->createRigidDynamic(PxTransform(physPot, physRot));
		// TODO: physx does not appear to handle objects with a small mass well; handle this properly rather than providing objects with a minimum of 10kg
		static_cast<PxRigidDynamic*>(body)->setMass(10 + mass);
	}

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

void GameObject::recalculateModelTransform() {
	modelTransform = glm::scale(glm::translate(glm::mat4(1.0f), position) * rotation, scaleVal);
	isDirty = false;
}

void GameObject::rotate(const float& angle, glm::vec3 const& axes) {
	rotation = glm::rotate(rotation, angle, axes);
	isDirty = true;
}

glm::quat GameObject::setRotation(const glm::vec3& rotationEA, const bool& fixInitialRotation) {
	// models loaded in from assimp are offset by 90 degrees (pi/2); we correct that here
	glm::quat q = glm::quat(fixInitialRotation ? glm::vec3(rotationEA.x + glm::half_pi<float>(),rotationEA.y,rotationEA.z) : rotationEA);
	rotation = glm::toMat4(q);
	return q;
}

void GameObject::translate(glm::vec3 const& amnt) {
	position += amnt;
	isDirty = true;
}

void GameObject::setPos(glm::vec3 const& newPos) {
	position = newPos;
	isDirty = true;
}

void GameObject::scale(glm::vec3 const& amnt) {
	scaleVal += amnt;
	isDirty = true;
}

void GameObject::setScale(glm::vec3 const& newScale) {
	scaleVal = newScale;
	isDirty = true;
}

void GameObject::update() {
	// update position and rotation to match the physics body
	if (usePhysics) {
		PxTransform pose = body->getGlobalPose();
		if (std::memcmp(&pose.p, &position, sizeof(glm::vec3)) != 0) {
			std::memcpy(&position, &pose.p, sizeof(glm::vec3));
			isDirty = true;
		}
		
		if (std::memcmp(&pose.q, &prevRot, sizeof(glm::vec4)) != 0) {
			std::memcpy(&prevRot, &pose.q, sizeof(glm::vec4));
			rotation = glm::toMat4(glm::quat(pose.q.w, pose.q.x, pose.q.y, pose.q.z));
			isDirty = true;
		}
	}
}