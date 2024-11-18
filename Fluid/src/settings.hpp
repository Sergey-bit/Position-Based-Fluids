#ifndef SETTINGS
#define SETTINGS

#include "math/minmath.hpp"

using scalar = float;

static constexpr unsigned WWIDTH  = 1080U;
static constexpr unsigned WHEIGHT = 720U;

static constexpr unsigned BOXWIDTH  = 400U;
static constexpr unsigned BOXHEIGHT = 200U;

static constexpr unsigned BOXMARGINX = (WWIDTH - BOXWIDTH) / 2;
static constexpr unsigned BOXMARGINY = (WHEIGHT - BOXHEIGHT) / 2;

static constexpr unsigned PARTICLES_NUMBER = 500U;

static constexpr scalar scale = 1.0f;
static constexpr float area = 16.0f;

static constexpr float influenceRadius = area / scale;
static constexpr float radius = 4.0f / scale;

static constexpr float interactionInputRadius = 5.0 * influenceRadius;

static constexpr scalar mass = 100.0f;
static constexpr scalar targetDensity = 0.001f;

static constexpr int threads = 40;

#endif