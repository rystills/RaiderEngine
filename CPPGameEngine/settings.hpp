#include "stdafx.h"
#pragma once
// this file holds global settings and shared engine data
unsigned int SCR_WIDTH = 1280, SCR_HEIGHT = 720;
bool useVsync = true;
bool fullScreen = false;
const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
bool debugDraw = false;

GLFWwindow* window;
#include "Player.hpp"
// TODO: we should not force usage of the Player base class
Player player;

#include "model.hpp"
#include "GameObject.hpp"
#include "Light.hpp"
std::unordered_map<std::string, std::shared_ptr<Model>> models;
std::vector<std::unique_ptr<GameObject>> gameObjects;
std::vector<std::unique_ptr<Light>> lights;
std::unordered_map<std::string,std::unique_ptr<Shader>> shaders;