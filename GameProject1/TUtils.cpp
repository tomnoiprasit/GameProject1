#include "TUtils.h"

TUtils::TUtils()
{
}


TUtils::~TUtils()
{
}

bool TUtils::intersects(D2D1_RECT_F rect1, D2D1_RECT_F rect2) {
	return !(rect1.right < rect2.left ||
			rect2.right < rect1.left ||
			rect1.bottom < rect2.top ||
			rect2.bottom < rect1.top);
}

int TUtils::randIntBetween(int begin, int end) {
	std::default_random_engine e(std::random_device{}());
	std::uniform_int_distribution<int> u(begin, end);
	return u(e);
}
