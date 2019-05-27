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
	btCollisionShape* trimeshShape;
	btRigidBody* body;
	int bodyIndex;

	GameObject(glm::vec3 position, glm::vec3 rotationEA, glm::vec3 scale, std::string modelName, bool isStaticMesh = false) : position(position), scale(scale) {
		//TODO: this should be simplified: the intermediate transformation into a quaternion seems to be overkill
		setRotation(rotationEA);
		std::unordered_map<std::string, std::shared_ptr<Model>>::iterator search = models.find(modelName);
		if (search != models.end())
			model = search->second;
		else {
			// TODO: don't use hard-coded model folder
			std::shared_ptr<Model> m(new Model(FileSystem::getPath("models/" + modelName + "/" + modelName + ".fbx")));
			models.insert({modelName, m});
			model = m;
		}

		// create bullet physics collider from model
		btTriangleMesh* trimesh = new btTriangleMesh();
		for (int j = 0; j < model->meshes.size(); ++j) {
			Mesh mesh = model->meshes[j];
			for (int i = 0; i < mesh.indices.size(); i += 3) {
				btVector3 vertex_1{ mesh.vertices[mesh.indices[i]].Position.x, mesh.vertices[mesh.indices[i]].Position.y, mesh.vertices[mesh.indices[i]].Position.z };
				btVector3 vertex_2{ mesh.vertices[mesh.indices[i+1]].Position.x, mesh.vertices[mesh.indices[i+1]].Position.y, mesh.vertices[mesh.indices[i+1]].Position.z };
				btVector3 vertex_3{ mesh.vertices[mesh.indices[i+2]].Position.x, mesh.vertices[mesh.indices[i+2]].Position.y, mesh.vertices[mesh.indices[i+2]].Position.z };
				trimesh->addTriangle(vertex_1, vertex_2, vertex_3);
			}
		}
		trimeshShape = new btBvhTriangleMeshShape{ trimesh, true };
		bulletData.collisionShapes.push_back(trimeshShape);

		// Create Dynamic Objects
		btTransform startTransform;
		startTransform.setIdentity();

		btScalar mass(isStaticMesh ? 0.0f : 1.0f);
		btVector3 localInertia(0, 0, 0);
		if (!isStaticMesh)
			trimeshShape->calculateLocalInertia(mass, localInertia);

		startTransform.setOrigin(btVector3(position.x,position.y,position.z));

		// using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
		btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, trimeshShape, localInertia);
		body = new btRigidBody(rbInfo);
		bodyIndex = bulletData.dynamicsWorld->getNumCollisionObjects();
		bulletData.dynamicsWorld->addRigidBody(body);
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
	}

	void setRotation(glm::vec3 rotationEA) {
		rotation = glm::toMat4(glm::quat(rotationEA));
	}
};
#endif