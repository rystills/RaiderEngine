#ifndef PLAYERSPAWN_H
#define PLAYERSPAWN_H

class PlayerSpawn {
public:
	PlayerSpawn(glm::vec3 position, glm::vec3 rotationEA) {
		camera.Position = position;
		camera.Yaw = glm::degrees(rotationEA.z);
		camera.updateCameraVectors();
	}
};
#endif
