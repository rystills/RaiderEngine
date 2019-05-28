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
#include <LinearMath/btGeometryUtil.h>

class GameObject {
public:
	glm::vec3 position;
	glm::mat4 rotation;
	glm::vec3 scale;
	std::shared_ptr<Model> model;

	// bullet data
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
			models.insert({ modelName, m });
			model = m;
		}

		// TODO: share collision shapes between gameObject instances
		if (isStaticMesh) {
			// create bullet physics collider from model
			btTriangleMesh* trimesh = new btTriangleMesh();
			for (int j = 0; j < model->meshes.size(); ++j) {
				Mesh mesh = model->meshes[j];
				for (int i = 0; i < mesh.indices.size(); i += 3) {
					btVector3 vertex_1{ mesh.vertices[mesh.indices[i]].Position.x, mesh.vertices[mesh.indices[i]].Position.y, mesh.vertices[mesh.indices[i]].Position.z };
					btVector3 vertex_2{ mesh.vertices[mesh.indices[i + 1]].Position.x, mesh.vertices[mesh.indices[i + 1]].Position.y, mesh.vertices[mesh.indices[i + 1]].Position.z };
					btVector3 vertex_3{ mesh.vertices[mesh.indices[i + 2]].Position.x, mesh.vertices[mesh.indices[i + 2]].Position.y, mesh.vertices[mesh.indices[i + 2]].Position.z };
					trimesh->addTriangle(vertex_1, vertex_2, vertex_3);
				}
			}
			btCollisionShape* trimeshShape = new btBvhTriangleMeshShape{ trimesh, true };
			bulletData.collisionShapes.push_back(trimeshShape);

			btVector3 btscale(scale.x, scale.y, scale.z);
			trimeshShape->setLocalScaling(btscale);
			trimeshShape->setMargin(0);
			bulletData.collisionShapes.push_back(trimeshShape);

			// Create Dynamic Objects
			btTransform startTransform;
			startTransform.setIdentity();

			btScalar mass(isStaticMesh ? 0.0f : 1.0f);
			btVector3 localInertia(0, 0, 0);
			if (!isStaticMesh)
				trimeshShape->calculateLocalInertia(mass, localInertia);
			startTransform.setOrigin(btVector3(position.x, position.y, position.z));

			btQuaternion quat;
			quat.setEulerZYX(rotationEA.z, rotationEA.y, rotationEA.x); //or quat.setEulerZYX depending on the ordering you want
			startTransform.setRotation(quat);

			// using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
			btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
			btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, trimeshShape, localInertia);
			body = new btRigidBody(rbInfo);
			bodyIndex = bulletData.dynamicsWorld->getNumCollisionObjects();
			bulletData.dynamicsWorld->addRigidBody(body);
		}
		else {
			// create bullet physics collider from model
			btAlignedObjectArray<btVector3> vertices;
			for (int j = 0; j < model->meshes.size(); ++j) {
				Mesh mesh = model->meshes[j];
				for (int i = 0; i < mesh.indices.size(); ++i) {
					btVector3 vertex{ mesh.vertices[mesh.indices[i]].Position.x, mesh.vertices[mesh.indices[i]].Position.y, mesh.vertices[mesh.indices[i]].Position.z };
					vertices.push_back(vertex);
				}
			}
			
			// shrink convex hull by margin to cancel it out (this isn't a perfect solution, but it works well enough - see https://pybullet.org/Bullet/phpBB3/viewtopic.php?t=2358)
			float collisionMargin = 0.04f;
			btAlignedObjectArray<btVector3> planeEquations;
			btGeometryUtil::getPlaneEquationsFromVertices(vertices, planeEquations);

			btAlignedObjectArray<btVector3> shiftedPlaneEquations;
			for (int p = 0; p<planeEquations.size(); ++p) {
				btVector3 plane = planeEquations[p];
				plane[3] += collisionMargin;
				shiftedPlaneEquations.push_back(plane);
			}
			btAlignedObjectArray<btVector3> shiftedVertices;
			btGeometryUtil::getVerticesFromPlaneEquations(shiftedPlaneEquations, shiftedVertices);

			btCollisionShape* convexShape = new btConvexHullShape(&(shiftedVertices[0].getX()), shiftedVertices.size());
			bulletData.collisionShapes.push_back(convexShape);

			btVector3 btscale(scale.x, scale.y, scale.z);
			convexShape->setLocalScaling(btscale);
			// since we shrink the convex hull by the margin independently of scaling, multiply the applied margin by the average scale to compensate
			// TODO: multiplying by the average scale won't work well for non-uniform scaled objects; such objects need their own convex mesh with baked per-axis margins
			convexShape->setMargin(collisionMargin*((scale.x + scale.y + scale.z) / 3));


			// Create Dynamic Objects
			btTransform startTransform;
			startTransform.setIdentity();

			btScalar mass(isStaticMesh ? 0.0f : 1.0f);
			btVector3 localInertia(0, 0, 0);
			if (!isStaticMesh)
				convexShape->calculateLocalInertia(mass, localInertia);
			startTransform.setOrigin(btVector3(position.x, position.y, position.z));

			btQuaternion quat;
			quat.setEulerZYX(rotationEA.z, rotationEA.y, rotationEA.x); //or quat.setEulerZYX depending on the ordering you want
			startTransform.setRotation(quat);

			// using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
			btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
			btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, convexShape, localInertia);
			body = new btRigidBody(rbInfo);
			bodyIndex = bulletData.dynamicsWorld->getNumCollisionObjects();
			bulletData.dynamicsWorld->addRigidBody(body);
		}
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
