#include "NPC2.h"


NPC2::NPC2()
{
}


NPC2::~NPC2()
{
}

void NPC2::setUp(ID2D1Bitmap* spriteSheet, std::vector<D2D1_RECT_F> sourceRectangle) {
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

void NPC2::update(GameTime gameTime) {

	float xIntPart;
	float yIntPart;

	if (status == 0) {
		stop();
	}
	else if (status == 1) {
		moveLeft();
	}
	else {
		moveRight();
	}

	modf(static_cast<float>(xLocation + velocity.getX() * 
		static_cast<float>(gameTime.getElapsedTime())), &xIntPart);
	modf(static_cast<float>(yLocation + velocity.getY() * 
		static_cast<float>(gameTime.getElapsedTime())), &yIntPart);
	
	setLocation(xIntPart, yIntPart);

	bounceLeftRight();
	bounceTopBottom();

	if (status == 0) {
		++idle;
		if (idle > 100) {
			// Change the status to moving again
			setStatus(TUtils::randIntBetween(1,2));
		}
	}

	++frameCount;
	if (frameCount > framePerAction) {
		++currentFrame;
		if (currentFrame == maxFrameForAction)
			currentFrame = 0;
		frameCount = 0;
	}

}

void NPC2::moveLeft() {
	if (velocity.getX() > 0) {
		velocity.setX(-1 * velocity.getX());
		velocity.setY(0);
	}
	else if (velocity.getX() == 0) {
		// Should be randomed
		velocity.setX(-3);
		velocity.setY(0);
	}
}
void NPC2::moveRight() {
	if (velocity.getX() < 0) {
		velocity.setX(-1 * velocity.getX());
		velocity.setY(0);
	}
	else if (velocity.getX() == 0) {
		// Should be randomed
		velocity.setX(3);
		velocity.setY(0);
	}
}
void NPC2::stop() {
	velocity.setX(0);
	velocity.setY(0);
}

void NPC2::setStatus(int i) {
	status = i;
	idle = 0;
}

void NPC2::bounceLeftRight() {
	float width = sourceRectangle.at(currentFrame).right - sourceRectangle.at(currentFrame).left + 1;
	if (drawRectangle.right > windowSize.right) {
		drawRectangle.left = windowSize.right - width;
		drawRectangle.right = drawRectangle.left + width;
		xLocation = drawRectangle.left + width / 2;
		setStatus(0);
	}
	else if (drawRectangle.left < 0) {
		drawRectangle.left = 0;
		drawRectangle.right = drawRectangle.left + width;
		xLocation = drawRectangle.left + width / 2;
		setStatus(0);
	}
}
