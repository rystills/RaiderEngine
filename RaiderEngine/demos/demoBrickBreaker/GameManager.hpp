#pragma once
#include "Paddle.hpp"
#include "Ball.hpp"
#include "TextObject.hpp"

inline int score = 0, level = 1;
inline bool paused = true;
#define levelMultiplier (1 + level * .2f)
inline Paddle* paddle;

class GameManager : public TextObject {
public:
	void restartGame(bool clearScore = true);
	void incrementLevel();
	Ball* ball;

	GameManager(std::string fontName, int fontSize);

	void update() override;

	void draw(Shader s) override;
};