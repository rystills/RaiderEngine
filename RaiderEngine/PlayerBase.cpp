#include "stdafx.h"
#include "camera.hpp"
#include "PlayerBase.hpp"
#include "physics.hpp"
#include "timing.hpp"
#include "settings.hpp"
#include "input.hpp"

void PlayerBase::init(float inHeight, float inRadius) {
	height = inHeight;
	radius = inRadius;
	// create the controller manager
	manager = PxCreateControllerManager(*gScene);
	// create the player controller
	PxCapsuleControllerDesc desc;
	desc.contactOffset = .01f;
	desc.height = height;
	desc.radius = radius;
	desc.material = gMaterial;
	desc.stepOffset = stepHeight;
	
	// TODO: player contactOffset has been reduced, but the player still hovers a good distance off of the ground; further exploration needed
	controller = (PxCapsuleController*)manager->createController(desc);
	// set the player controller's user data
	controller->setUserData(this);
	// set non-default raycast filter so that the player is ignored when raycasting
	unsigned int nbShapes = controller->getActor()->getNbShapes();

	PxShape** shapes = new PxShape * [nbShapes];
	controller->getActor()->getShapes(shapes, nbShapes, 0);
	for (unsigned int i = 0; i < nbShapes; ++i) {
		shapes[i]->setQueryFilterData(noHitFilterData);
	}
	
	// create a separate water check shape for determining whether or not the player is swimming (rather than just standing in a small puddle)
	waterCheckShape = gPhysics->createShape(PxSphereGeometry(waterCheckRadius), *gMaterial, false);
	waterCheckShape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);
	waterCheckShape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, false);
	waterCheckShape->setQueryFilterData(noHitFilterData);
	waterCheckBody = gPhysics->createRigidDynamic(PxTransform(0, 0, 0));
	waterCheckBody->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
	waterCheckBody->attachShape(*waterCheckShape);
	gScene->addActor(*waterCheckBody);

	// initialize the camera body to a very tiny sphere (basically a point) so we can tell when the camera is inside of a volume
	camShape = gPhysics->createShape(PxSphereGeometry(.00001f), *gMaterial, false);
	camShape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);
	camShape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, false);
	camShape->setQueryFilterData(noHitFilterData);
	camBody = gPhysics->createRigidDynamic(PxTransform(0,0,0));
	camBody->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
	camBody->attachShape(*camShape);
	gScene->addActor(*camBody);
}

void PlayerBase::setPos(glm::vec3 pos, bool relative, bool isFeetPos) {
	if (relative) {
		PxExtendedVec3 curPos = controller->getPosition();
		controller->setPosition(PxExtendedVec3(pos.x + curPos.x, pos.y + curPos.y + (isFeetPos ? height / 2 : 0), pos.z + curPos.z));
	}
	else
		controller->setPosition(PxExtendedVec3(pos.x, pos.y + (isFeetPos ? height / 2 : 0), pos.z));
	syncCameraPos();
	syncWaterCheckPos();
}

void PlayerBase::updateSwimmingVolumeCount(bool enteredNewBody) {
	swimmingVolumeCount += (enteredNewBody ? 1 : -1);
	swimming = swimmingVolumeCount > 0;
}

void PlayerBase::updateUnderWaterVolumeCount(bool enteredNewBody) {
	underWaterVolumeCount += (enteredNewBody ? 1 : -1);
	underWater = underWaterVolumeCount > 0;
}

void PlayerBase::updateLadderVolumeCount(bool enteredNewBody, GameObject* ladder) {
	ladderVolumeCount += (enteredNewBody ? 1 : -1);
	climbingLadder = ladderVolumeCount > 0;
	if (enteredNewBody)
		connectedLadder = ladder;
}

void PlayerBase::syncCameraPos() {
	PxExtendedVec3 playerPos = controller->getFootPosition();
	mainCam->Position.x = static_cast<float>(playerPos.x);
	// camera height should be set to the top of the capsule minus the approximate distance from the top of the head to the eyes
	mainCam->Position.y = static_cast<float>(playerPos.y) + height * (crouching ? crouchScale : 1) + 2*radius - eyeTopHeadOffset;
	mainCam->Position.z = static_cast<float>(playerPos.z);
	camBody->setGlobalPose(PxTransform(mainCam->Position.x, mainCam->Position.y, mainCam->Position.z));
}

void PlayerBase::syncWaterCheckPos() {
	PxExtendedVec3 footPos = controller->getFootPosition();
	waterCheckBody->setGlobalPose(PxTransform(footPos.x, footPos.y + waterCheckFootOffset + waterCheckRadius, footPos.z));
}

bool PlayerBase::canJump() {
	PxControllerState cctState;
	controller->getState(cctState);
	return ((cctState.collisionFlags & PxControllerCollisionFlag::eCOLLISION_DOWN) != 0) && (velocity.y <= 0);
}

void PlayerBase::update() {
	// toggle between player synced camera and free cam
	if (keyPressed("toggleFlyCam"))
		flyCam = !flyCam;

	if (climbingLadder) {
		// movement
		glm::vec4 effectiveVelocity;
		// jump
		if (keyHeld("jump")) {
			float forwardDir = (keyHeld("mvForward") - keyHeld("mvBackward"));
			float strafeDir = (keyHeld("mvRight") - keyHeld("mvLeft"));
			if (forwardDir && strafeDir) {
				// average forward and strafe speeds to prevent diagonal movement from being faster
				float invMag = 1 / sqrt(forwardDir * forwardDir + strafeDir * strafeDir);
				forwardDir *= invMag;
				strafeDir *= invMag;
			}
			// default to jumping forward if no direction is input
			else if (!(forwardDir || strafeDir))
				forwardDir = 1;
			glm::vec3 normalFront = glm::cross(mainCam->WorldUp, mainCam->Right);
			// TODO: should ladder jump forward/strafe speed be affected by crouch/run state?
			velocity = glm::vec3(normalFront * forwardDir * ladderJumpSpeed);
			velocity += glm::vec3(mainCam->Right * strafeDir * ladderJumpSpeed);
			velocity.y = ladderJumpStrength;
			effectiveVelocity = glm::vec4(velocity,1);
		}
		else {
			float baseMoveSpeed = (crouching ? crouchSpeed : walkSpeed), forwardSpeed = 0, strafeSpeed = 0;
			if (mainCam->controllable && !flyCam) {
				baseMoveSpeed = (crouching ? crouchSpeed : (keyHeld("run") ? runSpeed : walkSpeed));
				forwardSpeed = (keyHeld("mvForward") - keyHeld("mvBackward")) * baseMoveSpeed;
				strafeSpeed = (keyHeld("mvRight") - keyHeld("mvLeft")) * baseMoveSpeed;
				if (forwardSpeed && strafeSpeed) {
					// average forward and strafe speeds to prevent diagonal movement from being faster
					float invMag = baseMoveSpeed / sqrt(forwardSpeed * forwardSpeed + strafeSpeed * strafeSpeed);
					forwardSpeed *= invMag;
					strafeSpeed *= invMag;
				}
			}
			velocity += mainCam->Front * forwardSpeed * ladderControl * deltaTime;
			velocity += mainCam->Right * strafeSpeed * ladderControl * deltaTime;
			// cap horizontal velocity depending on whether the player is walking or running
			float maxMoveSpeed = baseMoveSpeed * maxMoveSpeedRatio;
			float moveVel = sqrt(velocity.x * velocity.x + velocity.y * velocity.y + velocity.z * velocity.z);
			if (moveVel > maxMoveSpeed) {
				float velDiff = (maxMoveSpeed) / moveVel;
				velocity.x *= velDiff;
				velocity.y *= velDiff;
				velocity.z *= velDiff;
			}
			// if the player is not inputting movement in any direction, slow them down or stop them entirely
			if (forwardSpeed == 0 && strafeSpeed == 0) {
				float stopSpeed = ladderStoppingSpeed * deltaTime;
				moveVel = sqrt(velocity.x * velocity.x + velocity.y * velocity.y + velocity.z * velocity.z);
				if (moveVel <= stopSpeed) {
					velocity = glm::vec3();
				}
				else {
					velocity -= glm::normalize(velocity) * ladderStoppingSpeed * deltaTime;
				}
			}

			bool grounded = canJump();
			// restrict effective velocity to ladder normal when not grounded
			effectiveVelocity = glm::vec4(velocity, 1);
			if (!grounded) {
				// rotate velocity by ladder rotation to get axis alignment
				glm::quat q = connectedLadder->rotation;
				// add a 90 degree rotation on the x axis to compensate for the -90 degree offset produced by assimp
				q *= glm::angleAxis(glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f));
				effectiveVelocity = q * effectiveVelocity;
				// eliminate z to restrict movement to the xy plane, then rotate resulting velocity back by inverse ladder rotation
				effectiveVelocity.z = 0;
				effectiveVelocity = glm::inverse(q) * effectiveVelocity;
			}
		}
		// apply to controller
		PxVec3 physVelocity(effectiveVelocity.x * deltaTime, effectiveVelocity.y * deltaTime, effectiveVelocity.z * deltaTime);
		controller->move(physVelocity, 0, deltaTime, NULL);
	}

	else if (swimming) {
		// movement
		float baseMoveSpeed = (crouching ? crouchSpeed : walkSpeed), forwardSpeed = 0, strafeSpeed = 0;
		if (mainCam->controllable && !flyCam) {
			baseMoveSpeed = (crouching ? crouchSpeed : (keyHeld("run") ? runSpeed : walkSpeed));
			forwardSpeed = (keyHeld("mvForward") - keyHeld("mvBackward")) * baseMoveSpeed;
			strafeSpeed = (keyHeld("mvRight") - keyHeld("mvLeft")) * baseMoveSpeed;
			if (forwardSpeed && strafeSpeed) {
				// average forward and strafe speeds to prevent diagonal movement from being faster
				float invMag = baseMoveSpeed / sqrt(forwardSpeed * forwardSpeed + strafeSpeed * strafeSpeed);
				forwardSpeed *= invMag;
				strafeSpeed *= invMag;
			}
		}
		velocity += mainCam->Front * forwardSpeed * waterControl * deltaTime;
		velocity += mainCam->Right * strafeSpeed * waterControl * deltaTime;
		// cap horizontal velocity depending on whether the player is walking or running
		float maxMoveSpeed = baseMoveSpeed * maxMoveSpeedRatio;
		float moveVel = sqrt(velocity.x * velocity.x + velocity.y * velocity.y + velocity.z * velocity.z);
		if (moveVel > maxMoveSpeed) {
			float velDiff = (maxMoveSpeed) / moveVel;
			velocity.x *= velDiff;
			velocity.y *= velDiff;
			velocity.z *= velDiff;
		}
		// if the player is not inputting movement in any direction, slow them down or stop them entirely
		if (forwardSpeed == 0 && strafeSpeed == 0) {
			float stopSpeed = waterStoppingSpeed * deltaTime;
			moveVel = sqrt(velocity.x * velocity.x + velocity.y * velocity.y + velocity.z * velocity.z);
			if (moveVel <= stopSpeed) {
				velocity = glm::vec3();
			}
			else {
				velocity -= glm::normalize(velocity) * waterStoppingSpeed * deltaTime;
			}
		}
		// apply to controller
		PxVec3 physVelocity(velocity.x * deltaTime, velocity.y * deltaTime, velocity.z * deltaTime);
		controller->move(physVelocity, 0, deltaTime, NULL);
	}
	else {
		glm::vec3 normalFront = glm::cross(mainCam->WorldUp, mainCam->Right);
		bool grounded = canJump();
		// movement
		float baseMoveSpeed = (crouching ? crouchSpeed : walkSpeed), forwardSpeed = 0, strafeSpeed = 0;
		if (mainCam->controllable && !flyCam) {
			baseMoveSpeed = (crouching ? crouchSpeed : (keyHeld("run") ? runSpeed : walkSpeed));
			forwardSpeed = (keyHeld("mvForward") - keyHeld("mvBackward")) * baseMoveSpeed;
			strafeSpeed = (keyHeld("mvRight") - keyHeld("mvLeft")) * baseMoveSpeed;
			if (forwardSpeed && strafeSpeed) {
				// average forward and strafe speeds to prevent diagonal movement from being faster
				float invMag = baseMoveSpeed / sqrt(forwardSpeed * forwardSpeed + strafeSpeed * strafeSpeed);
				forwardSpeed *= invMag;
				strafeSpeed *= invMag;
			}
		}
		velocity += normalFront * forwardSpeed * (grounded ? 1 : airControl) * deltaTime;
		velocity += mainCam->Right * strafeSpeed * (grounded ? 1 : airControl) * deltaTime;
		// As long as we're grounded, keep vertical velocity at a few ticks of gravity. If we're airborn, continually apply gravity until we return to the ground
		// TODO: keep the player tethered to slopes / steps without relying on an artificial gravity
		if (grounded)
			velocity.y = -playerGravity * 10 * deltaTime;
		else
			velocity.y -= playerGravity * deltaTime;

		// cap horizontal velocity depending on whether the player is walking or running
		float maxMoveSpeed = baseMoveSpeed * maxMoveSpeedRatio;
		float moveVel = sqrt(velocity.x * velocity.x + velocity.z * velocity.z);
		if (moveVel > maxMoveSpeed) {
			float velDiff = (maxMoveSpeed) / moveVel;
			velocity.x *= velDiff;
			velocity.z *= velDiff;
		}
		// if the player is not inputting movement in any direction, slow them down or stop them entirely
		if (forwardSpeed == 0 && strafeSpeed == 0) {
			float stopSpeed = (grounded ? groundStoppingSpeed : airStoppingSpeed) * deltaTime;
			moveVel = sqrt(velocity.x * velocity.x + velocity.z * velocity.z);
			if (moveVel <= stopSpeed) {
				velocity.x = 0;
				velocity.z = 0;
			}
			else {
				float velDir = std::atan2(velocity.z, velocity.x);
				velocity.x -= cos(velDir) * stopSpeed;
				velocity.z -= sin(velDir) * stopSpeed;
			}
		}
		// jump
		if (mainCam->controllable)
			if (grounded && keyHeld("jump"))
				// jump velocity is a burst, so deltaTime is ignored
				velocity.y = jumpStrength;

		// stop moving up if we hit our head on a ceiling
		if (velocity.y > 0) {
			PxSceneReadLock scopedLock(*gScene);
			PxCapsuleGeometry geom(radius, height * (crouching ? crouchScale : 1) * .5f);
			PxExtendedVec3 position = controller->getPosition();
			PxVec3 pos((float)position.x, (float)position.y + headBumpDist, (float)position.z);
			PxQuat orientation(PxHalfPi, PxVec3(0.0f, 0.0f, 1.0f));

			PxOverlapBuffer hit;
			if (gScene->overlap(geom, PxTransform(pos, orientation), hit, PxQueryFilterData(defaultFilterData, PxQueryFlag::eANY_HIT | PxQueryFlag::eSTATIC | PxQueryFlag::eDYNAMIC)))
				velocity.y = 0;
		}
		// apply to controller
		PxVec3 physVelocity(velocity.x* deltaTime, velocity.y* deltaTime, velocity.z* deltaTime);
		controller->move(physVelocity, 0, deltaTime, NULL);
	}

	// crouch toggle
	if (keyPressed("crouch")) {
		// we can always crouch, but if we're trying to stand back up, make sure we won't hit our head on something
		if (crouching) {
			PxSceneReadLock scopedLock(*gScene);
			PxCapsuleGeometry geom(radius, height / 2);
			PxExtendedVec3 position = controller->getPosition();
			// take half of the difference between our standing height and our crouch height to get the amount we need to move up
			PxVec3 pos((float)position.x, (float)position.y + ((height - height * crouchScale) / 2), (float)position.z);
			PxQuat orientation(PxHalfPi, PxVec3(0.0f, 0.0f, 1.0f));

			PxOverlapBuffer hit;
			if (!gScene->overlap(geom, PxTransform(pos, orientation), hit, PxQueryFilterData(defaultFilterData, PxQueryFlag::eANY_HIT | PxQueryFlag::eSTATIC | PxQueryFlag::eDYNAMIC)))
				goto toggleCrouch;
		}
		else {
		toggleCrouch:
			crouching = !crouching;
			controller->resize(height * (crouching ? crouchScale : 1));
		}
	}
	syncWaterCheckPos();

	// update camera
	if (flyCam)
		mainCam->moveFlycam();
	else
		syncCameraPos();
	mainCam->updateViewProj();
}