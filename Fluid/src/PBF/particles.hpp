#ifndef PARTICLES
#define PARTICLES

#include "../math/minmath.hpp"
#include "../settings.hpp"
#include "../NearestNeighborSearch/segments.hpp"
#include "../math/kernelFunctions.hpp"
#include "../Debug/prints.hpp"
#include "../Debug/timer.hpp"

#include <random>
#include <iostream>
#include <array>
#include <time.h>
#include <omp.h>

struct Particle
{
	alignas(64) Point& center;
	alignas(64) vec2& dir;
};
struct Particles
{
	alignas(64) Point centers[PARTICLES_NUMBER];
	alignas(64) vec2 dir[PARTICLES_NUMBER];

	Particle operator[](int i)
	{
		return Particle{ centers[i], dir[i] };
	}
};

extern Particles particles;

extern alignas(64) float lambdas[PARTICLES_NUMBER];
extern alignas(64) float interactionInputStrength;
extern alignas(64) vec2  interactionInputPoint;

void newUpdateSegment(const int& i, const int& pre, const int& post);
void collisionResponse(const vec2& pos, const int& Index);
void boundaryCondition(const int& Index, vec2& dp);
void collisionHandler(const int& Index, vec2& dp);
void calcLambda(const int& Index);
void particlesUpdate();
void initParticles();
void distribute();
void sort();

vec2 calcDeltaPosition(const int& Index);
vec2 calcGradient(const int& Index, const int& k);
vec2 calcVorticityAndViscosity(const int& Index);
vec2 ExternalForces(const vec2& pos, const vec2& velocity);

#endif