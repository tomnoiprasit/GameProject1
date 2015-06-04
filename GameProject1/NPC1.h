#pragma once
#include <random>
#include <math.h>
#include "NonPlayer.h"
#define _USE_MATH_DEFINES

class NPC1 : public NonPlayer
{
	// NPC1 specific
	int status = 0;
	D2D1_POINT_2F pivotalPoint;
	float angle = 0; // in degree
	int frameToUpdateAngle = 5; // arbitary number
	int angleFrameCount = 0;
	float radius = 100.f;
public:
	NPC1();
	~NPC1();
	void setUp(ID2D1Bitmap*, std::vector<D2D1_RECT_F>);
	void update(GameTime gameTime);

	void moveInCircle(GameTime gameTime);
	void moveInRandom(GameTime gameTime);
	void setPivotalPoint(D2D1_POINT_2F);
	void setFrametoUpdateAngle(int);
	void setRadius(float);
	void toggleStatus();
	void setRandomAngle();

};

