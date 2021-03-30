#include "conversionHelpers.hpp"

glm::vec3 pxToGlmVec3(physx::PxExtendedVec3 v)
{
	return glm::vec3(v.x, v.y, v.z);
}

physx::PxExtendedVec3 glmToPxVec3(glm::vec3 v)
{
	return physx::PxExtendedVec3(v.x,v.y,v.z);
}