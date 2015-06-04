#pragma once
#include <sstream>
#include "NonPlayer.h"
#include "TUtils.h"

class NPC2 : public NonPlayer
{
	// NPC2 specific
	int status = 0;
	int idle = 0;

public:
	NPC2();
	~NPC2();
	// Override
	void setUp(ID2D1Bitmap*, 
		std::vector<D2D1_RECT_F>);
	void update(GameTime);
	void bounceLeftRight();
	// Specific
	void moveLeft();
	void moveRight();
	void stop();
	void setStatus(int);
};

