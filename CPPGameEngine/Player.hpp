#ifndef PLAYER_H
#define PLAYER_H
#include "camera.hpp"

class Player {
public:
	Camera camera;
	Player() : camera(glm::vec3(0)) {
	}
};
#endif