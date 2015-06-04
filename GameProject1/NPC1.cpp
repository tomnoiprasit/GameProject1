#include "NPC1.h"


NPC1::NPC1()
{
	maxFrameForAction = 1;
	framePerAction = 1;
	frameCount = 0;
	randomSpeed();
	xLocation = 0;
	yLocation = 0;
	active = true;
}

NPC1::~NPC1()
{
}

void NPC1::setUp(ID2D1Bitmap* spriteSheet, std::vector<D2D1_RECT_F> sourceRectangle) {
	// Default window size
	// If not using default, subclass should call setWindowSize()
	windowSize = { 0, 0, 800, 600 };

	this->spriteSheet = spriteSheet;
	this->sourceRectangle = sourceRectangle;

	// Default to Frame 0
	setDrawRectangle(0, 0, sourceRectangle.at(0).right - sourceRectangle.at(0).left, sourceRectangle.at(0).bottom - sourceRectangle.at(0).top);

	currentFrame = 0;
	// maxFrameForAction should be derived from sourceRectangle vector
	// 
	maxFrameForAction = 1;
	framePerAction = 10;
	frameCount = 0;

	randomSpeed();

	float width = sourceRectangle.at(currentFrame).right - sourceRectangle.at(currentFrame).left;
	float height = sourceRectangle.at(currentFrame).bottom - sourceRectangle.at(currentFrame).top;

	xLocation = drawRectangle.left + width / 2;
	yLocation = drawRectangle.top + height / 2;

	active = true;
}

void NPC1::update(GameTime gameTime) {

	if (status == 0)
		moveInCircle(gameTime);
	else
		moveInRandom(gameTime);

	bounceLeftRight();
	bounceTopBottom();

	angleFrameCount++;
	if (angleFrameCount > frameToUpdateAngle) {
		angle++;
		if (angle > 359) angle = 0;
		angleFrameCount = 0;
	}

	++frameCount;
	if (frameCount > framePerAction) {
		++currentFrame;
		if (currentFrame == maxFrameForAction)
			currentFrame = 0;
		frameCount = 0;
	}

}

void NPC1::moveInCircle(GameTime gameTime) {
	float xIntPart;
	float yIntPart;
	xLocation = pivotalPoint.x + radius * static_cast<float>(sin(angle * 3.141592653589793238462643383279502884 / 180));
	yLocation = pivotalPoint.y + radius * static_cast<float>(cos(angle * 3.141592653589793238462643383279502884 / 180));
	modf(xLocation, &xIntPart);
	modf(yLocation, &yIntPart);
	setLocation(xIntPart, yIntPart);
}

void NPC1::moveInRandom(GameTime gameTime) {
	float xIntPart;
	float yIntPart;
	modf(static_cast<float>(xLocation + velocity.getX() * static_cast<float>(gameTime.getElapsedTime())), &xIntPart);
	modf(static_cast<float>(yLocation + velocity.getY() * static_cast<float>(gameTime.getElapsedTime())), &yIntPart);
	setLocation(xIntPart, yIntPart);
}

void NPC1::setPivotalPoint(D2D1_POINT_2F thePoint) {
	pivotalPoint = thePoint;
}

void NPC1::setFrametoUpdateAngle(int i) {
	frameToUpdateAngle = i;
}

void NPC1::setRadius(float f) {
	radius = f;
}

void NPC1::toggleStatus() {
	if (status == 0) status = 1;
	else status = 0;
}

void NPC1::setRandomAngle() {
	std::default_random_engine e(std::random_device{}());
	std::uniform_int_distribution<int> u(0, 359);
	angle = static_cast<float>(u(e));
}