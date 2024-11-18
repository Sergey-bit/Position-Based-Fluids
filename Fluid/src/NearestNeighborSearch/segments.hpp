#ifndef SEGMENT_S
#define SEGMENT_S

#include "../settings.hpp"

inline constexpr unsigned staticCeil(float d)
{
	unsigned val = d;
	return val + 1;
}

using node_t = int;

static constexpr unsigned cells_x = staticCeil(BOXWIDTH * 1.0f / area);
static constexpr unsigned cells_y = staticCeil(BOXHEIGHT * 1.0f / area);

static constexpr unsigned cellsSize = cells_x * cells_y;

struct Node
{
	node_t value = -1;
	Node* next = nullptr;
	Node* prev = nullptr;
};
struct List
{
	Node indices[PARTICLES_NUMBER];
	Node* segments[cellsSize] = { 0 };
};

int GetSegmentIndex(const vec2& pos);
int GetLocationFromShift(const unsigned& loc, const unsigned& shift);

#endif