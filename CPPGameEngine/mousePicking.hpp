#include <dVector.h>
#include <dMatrix.h>
#include <Newton.h>
#include <dNewton.h>
#include <dNewtonCollision.h>
#include <dNewtonDynamicBody.h>
#include <utility>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
extern float lastX, lastY;
extern bool mouseHeldLeft;

/*
convert a pair of x,y NDC coordinates to a world start and end position for raycasting
@param projection: the projection matrix to cast frrom
@param view: the view ematrix to cast from
@param x: the x coordinate of the raycast (in normalized device coordinate space)
@param y: the y coordinate of the raycast (in normalized device coordinate space)
@returns: a pair of dVectors representing the world start and end position, respectively
*/
std::pair<dVector, dVector> screenToWorld(glm::mat4 view, glm::mat4 projection, float x=0, float y=0) {
	glm::vec4 lRayStart_NDC(x, y, 0, 1.0f);
	// .94 for max object pick distance
	glm::vec4 lRayEnd_NDC(x, y, .94, 1.0f);

	// inverse transform matrices to camera space
	glm::mat4 M = glm::inverse(projection*view);
	glm::vec4 lRayStart_world = M * lRayStart_NDC; lRayStart_world /= lRayStart_world.w;
	glm::vec4 lRayEnd_world = M * lRayEnd_NDC; lRayEnd_world /= lRayEnd_world.w;
	return std::pair<dVector, dVector> {dVector(lRayStart_world.x, lRayStart_world.y, lRayStart_world.z), dVector(lRayEnd_world.x, lRayEnd_world.y, lRayEnd_world.z)};
}

void CalculatePickForceAndTorque(const NewtonBody* const body, const dVector& pointOnBodyInGlobalSpace, const dVector& targetPositionInGlobalSpace, dFloat timestep)
{
	dMatrix matrix;
	dVector com(0.0f);
	dVector omega0(0.0f);
	dVector veloc0(0.0f);
	dVector omega1(0.0f);
	dVector veloc1(0.0f);
	dVector pointVeloc(0.0f);

	const dFloat stiffness = 0.3f;
	const dFloat angularDamp = 0.95f;

	dFloat invTimeStep = 1.0f / timestep;
	NewtonWorld* const world = NewtonBodyGetWorld(body);
	NewtonWorldCriticalSectionLock(world, 0);

	// calculate the desired impulse
	NewtonBodyGetMatrix(body, &matrix[0][0]);
	NewtonBodyGetOmega(body, &omega0[0]);
	NewtonBodyGetVelocity(body, &veloc0[0]);

	NewtonBodyGetPointVelocity(body, &pointOnBodyInGlobalSpace[0], &pointVeloc[0]);

	dVector deltaVeloc(targetPositionInGlobalSpace - pointOnBodyInGlobalSpace);
	deltaVeloc = deltaVeloc.Scale(stiffness * invTimeStep) - pointVeloc;
	for (int i = 0; i < 3; i++) {
		dVector veloc(0.0f);
		veloc[i] = deltaVeloc[i];
		// todo: consider timestep (deltaTime) dependent impulse time
		// todo: consider enforcing a maximum impulse strength to minimize the risk of clipping
		// todo: rather than using ray end as target position, perform a second raycast from held object's position to ray end to respect walls and floors
		NewtonBodyAddImpulse(body, &veloc[0], &pointOnBodyInGlobalSpace[0], .15f);
	}

	// damp angular velocity
	NewtonBodyGetOmega(body, &omega1[0]);
	NewtonBodyGetVelocity(body, &veloc1[0]);
	omega1 = omega1.Scale(angularDamp);

	// restore body velocity and angular velocity
	NewtonBodySetOmega(body, &omega0[0]);
	NewtonBodySetVelocity(body, &veloc0[0]);

	// convert the delta velocity change to a external force and torque
	dFloat Ixx;
	dFloat Iyy;
	dFloat Izz;
	dFloat mass;
	NewtonBodyGetMass(body, &mass, &Ixx, &Iyy, &Izz);

	dVector angularMomentum(Ixx, Iyy, Izz);
	angularMomentum = matrix.RotateVector(angularMomentum.CrossProduct(matrix.UnrotateVector(omega1 - omega0)));

	dVector force((veloc1 - veloc0).Scale(mass * invTimeStep));
	dVector torque(angularMomentum.Scale(invTimeStep));

	NewtonBodyAddForce(body, &force[0]);
	NewtonBodyAddTorque(body, &torque[0]);

	// make sure the body is unfrozen, if it is picked
	NewtonBodySetSleepState(body, 0);

	NewtonWorldCriticalSectionUnlock(world);
}

class dMousePickClass
{
public:
	dMousePickClass()
		:m_param(1.0f)
		, m_body(NULL)
	{
	}

	// implement a ray cast pre-filter
	static unsigned RayCastPrefilter(const NewtonBody* body, const NewtonCollision* const collision, void* const userData)
	{
		// ray cannot pick trigger volumes
		//return NewtonCollisionIsTriggerVolume(collision) ? 0 : 1;

		const NewtonCollision* const parent = NewtonCollisionGetParentInstance(collision);
		if (parent) {
			// you can use this to filter sub collision shapes.  
			dAssert(NewtonCollisionGetSubCollisionHandle(collision));
		}

		return (NewtonBodyGetType(body) == NEWTON_DYNAMIC_BODY) ? 1 : 0;
	}

	static dFloat RayCastFilter(const NewtonBody* const body, const NewtonCollision* const collisionHit, const dFloat* const contact, const dFloat* const normal, dLong collisionID, void* const userData, dFloat intersetParam)
	{
		dFloat mass;
		dFloat Ixx;
		dFloat Iyy;
		dFloat Izz;

		// check if we are hitting a sub shape
		const NewtonCollision* const parent = NewtonCollisionGetParentInstance(collisionHit);
		if (parent) {
			// you can use this to filter sub collision shapes.  
			dAssert(NewtonCollisionGetSubCollisionHandle(collisionHit));
		}

		dMousePickClass* const data = (dMousePickClass*)userData;
		NewtonBodyGetMass(body, &mass, &Ixx, &Iyy, &Izz);
		if ((mass > 0.0f) || (NewtonBodyGetType(body) == NEWTON_KINEMATIC_BODY)) {
			data->m_body = body;
		}


		if (intersetParam < data->m_param) {
			data->m_param = intersetParam;
			data->m_normal = dVector(normal[0], normal[1], normal[2]);
		}
		return intersetParam;
	}

	dVector m_normal;
	dFloat m_param;
	const NewtonBody* m_body;
};


NewtonBody* MousePickByForce(NewtonWorld* const nWorld, const dVector& origin, const dVector& end, dFloat& paramterOut, dVector& positionOut, dVector& normalOut)
{
	dMousePickClass rayCast;
	NewtonWorldRayCast(nWorld, &origin[0], &end[0], dMousePickClass::RayCastFilter, &rayCast, dMousePickClass::RayCastPrefilter, 0);
	if (rayCast.m_body) {
		positionOut = origin + (end - origin).Scale(rayCast.m_param);
		normalOut = rayCast.m_normal;
		paramterOut = rayCast.m_param;
	}
	return (NewtonBody*)rayCast.m_body;
}

NewtonBody* m_targetPicked = NULL;
bool m_prevMouseState = false;
float m_pickedBodyParam;
dVector m_pickedBodyLocalAtachmentPoint;
dVector m_pickedBodyLocalAtachmentNormal;
dVector m_pickedBodyTargetPosition;
void UpdatePickBody(dFloat timestep, glm::mat4 view, glm::mat4 projection) {
	// handle pick body from the screen
	bool mousePickState = mouseHeldLeft;
	float m_mousePosX = lastX;
	float m_mousePosY = lastY;
	if (!m_targetPicked) {
		if (!m_prevMouseState && mousePickState) {
			dFloat param;
			dVector posit;
			dVector normal;

			std::pair<dVector, dVector> worldPoints = screenToWorld(view, projection);

			NewtonBody* const body = MousePickByForce(world, worldPoints.first, worldPoints.second, param, posit, normal);
			if (body) {
				m_targetPicked = body;
				dMatrix matrix;
				NewtonBodyGetMatrix(m_targetPicked, &matrix[0][0]);

				// save point local to the body matrix
				m_pickedBodyParam = param;
				m_pickedBodyLocalAtachmentPoint = matrix.UntransformVector(posit);

				// convert normal to local space
				m_pickedBodyLocalAtachmentNormal = matrix.UnrotateVector(normal);

				// link the a destructor callback
				//m_bodyDestructor = NewtonBodyGetDestructorCallback(m_targetPicked);
				//NewtonBodySetDestructorCallback(m_targetPicked, OnPickedBodyDestroyedNotify);
			}
		}

	}
	else {
		if (mousePickState) {
			dFloat x = dFloat(m_mousePosX);
			dFloat y = dFloat(m_mousePosY);
			std::pair<dVector, dVector> worldPoints = screenToWorld(view, projection);
			m_pickedBodyTargetPosition = worldPoints.first + (worldPoints.second - worldPoints.first).Scale(m_pickedBodyParam);

			dMatrix matrix;
			NewtonBodyGetMatrix(m_targetPicked, &matrix[0][0]);
			dVector point(matrix.TransformVector(m_pickedBodyLocalAtachmentPoint));
			CalculatePickForceAndTorque(m_targetPicked, point, m_pickedBodyTargetPosition, timestep);
		}
		else {
			if (m_targetPicked) {
				NewtonBodySetSleepState(m_targetPicked, 0);
			}

			// unchain the callbacks
			//NewtonBodySetDestructorCallback(m_targetPicked, m_bodyDestructor);
			m_targetPicked = NULL;
			//m_bodyDestructor = NULL;
		}
	}

	m_prevMouseState = mousePickState;
}