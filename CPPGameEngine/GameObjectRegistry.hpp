#ifndef GAME_OBJECT_REGISTRY_H
#define GAME_OBJECT_REGISTRY_H
#include "MovingPlatform.hpp"

template <typename ... TArgs>
void instantiateGameObject(std::string name, TArgs ... args) {
	if (name == "MovingPlatform") gameObjects.emplace_back(new MovingPlatform(args ...));
}

#endif