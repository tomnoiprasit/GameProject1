#pragma once
#include <d2d1.h>
#include <random>

class TUtils
{
public:
	TUtils();
	~TUtils();

	static bool intersects(D2D1_RECT_F, D2D1_RECT_F);
	static int randIntBetween(int, int);
	static std::wstring TUtils::toWSString(std::string s);
};

