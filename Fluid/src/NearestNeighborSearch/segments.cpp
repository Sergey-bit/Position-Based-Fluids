#include "segments.hpp"

int GetSegmentIndex(const vec2& pos)
{
	int x = glm::floor((pos.x - BOXMARGINX) * scale / area);
	int y = glm::floor((pos.y - BOXMARGINY) * scale / area);

	return y * cells_x + x;
}
int GetLocationFromShift(const unsigned& loc, const unsigned& shift)
{
	int ret = loc + (cells_x * (shift / 3 - 1)) - 1 + shift % 3;
	return (ret >= cellsSize) ? -1 : ret;
}