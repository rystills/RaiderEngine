#pragma once
#include "stdafx.h"

struct Direction {
	static const inline glm::vec3
		up = glm::vec3(0, 1, 0),
		down = glm::vec3(0, -1, 0),
		left = glm::vec3(-1, 0, 0),
		right = glm::vec3(1, 0, 0),
		forward = glm::vec3(0, 0, 1),
		back = glm::vec3(0, 0, -1);
};

struct Direction2D {
	static const inline glm::vec2
		up = glm::vec2(0, 1),
		down = glm::vec2(0, -1),
		left = glm::vec2(-1, 0),
		right = glm::vec2(1, 0);
};

// NOTE: colors derived from wikipedia's list of common color names
struct Color {
	static const inline glm::vec3
		black = glm::vec3(0),
		white = glm::vec3(1),
		red = glm::vec3(1, 0, 0),
		green = glm::vec3(0, 1, 0),
		blue = glm::vec3(0, 0, 1),
		orange = glm::vec3(1.0f, 0.4f, 0.0f),
		yellow = glm::vec3(1.0f, 1.0f, 0.0f),
		purple = glm::vec3(0.415686f, 0.050980f, 0.678431f),
		brown = glm::vec3(0.588235f, 0.294118f, 0.0f),
		magenta = glm::vec3(1.0f, 0.0f, 1.0f),
		tan = glm::vec3(0.823529f, 0.705882f, 0.549020f),
		cyan = glm::vec3(0.0f, 1.0f, 1.0f),
		olive = glm::vec3(0.501961f, 0.501961f, 0.0f),
		maroon = glm::vec3(0.501961f, 0.0f, 0.0f),
		navy = glm::vec3(0.0f, 0.0f, 0.501961f),
		aquamarine = glm::vec3(0.498039f, 1.0f, 0.831373f),
		turquoise = glm::vec3(0.250980f, 0.878431f, 0.815686f),
		silver = glm::vec3(0.752941f, 0.752941f, 0.752941f),
		darkGreen = glm::vec3(0.0f, 0.501961f, 0.0f),
		teal = glm::vec3(0.0f, 0.501961f, 0.501961f),
		indigo = glm::vec3(0.294118f, 0.0f, 0.509804f),
		violet = glm::vec3(0.498039f, 0.0f, 1.0f),
		pink = glm::vec3(1.0f, 0.752941f, 0.796078f),
		gray = glm::vec3(0.501961f, 0.501961f, 0.501961f);
};