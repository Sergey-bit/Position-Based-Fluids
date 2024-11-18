#include "particles.hpp"

static constexpr float coeff       = 1.0f / scale;
static constexpr float epsilon     = 0.000001f;
static constexpr float dt          = 0.1f;
static constexpr float resistance  = 0.9f;
static constexpr float gravity     = 30.0f;
static constexpr float viscosity_c = 0.04f;
static constexpr float relaxation  = 3e-6f;
static constexpr float delta_q     = 0.03f;
static constexpr int   iterations  = 20;

static constexpr float collision_penalty      = 0.01f;
static constexpr float tensible_instability_k = 0.1f;
static constexpr float tensible_instability_n = 4.0f;

alignas(64) Particles particles;
alignas(64) List       segments;

alignas(64) vec2 prediction[PARTICLES_NUMBER];
alignas(64) vec2   external[PARTICLES_NUMBER];

alignas(64) float   lambdas[PARTICLES_NUMBER];

alignas(64) vec2     interactionInputPoint(0.0, 0.0);
alignas(64) float    interactionInputStrength = 0.0 ;

void particlesUpdate()
{
	//Timer global; global.emerge();

	#pragma omp parallel num_threads(threads)
	{
		#pragma omp for schedule(dynamic, PARTICLES_NUMBER / threads)
		for (int i = 0; i < PARTICLES_NUMBER; ++i)
		{
			external[i] *= resistance;
			external[i] += ExternalForces(particles.centers[i], particles.dir[i]) * dt;

			particles.dir[i] += external[i];
			prediction[i] = particles.centers[i];

			vec2 shift = particles.dir[i] * dt;
			collisionHandler(i, shift);

			prediction[i] += shift;
			newUpdateSegment(
				i, GetSegmentIndex(particles.centers[i]), GetSegmentIndex(prediction[i])
			);

		}

		for (int j = 0; j < iterations; ++j)
		{
			// Fill arrays values
			#pragma omp for schedule(dynamic, PARTICLES_NUMBER / (2 * threads))
			for (int i = 0; i < PARTICLES_NUMBER; ++i)
			{
				calcLambda(i);
			}

			// Compute position shift
			#pragma omp for schedule(dynamic, PARTICLES_NUMBER / (2 * threads))
			for (int i = 0; i < PARTICLES_NUMBER; ++i)
			{
				vec2 deltaPosition = calcDeltaPosition(i);
				int unit = GetSegmentIndex(prediction[i]);

				collisionHandler(i, deltaPosition);
				prediction[i] += deltaPosition;

				newUpdateSegment(
					i, unit,
					GetSegmentIndex(prediction[i])
				);
			}
		}

		#pragma omp for schedule(dynamic, PARTICLES_NUMBER / (4 * threads))
		for (int i = 0; i < PARTICLES_NUMBER; ++i)
		{
			vec2 force = calcVorticityAndViscosity(i);
			particles.dir[i] = (prediction[i] - particles.centers[i]) * dt;

			particles.dir[i] += force;
			particles.centers[i] = prediction[i];

		}
	}

	//DBG::print(global.done(), "");
}
void newUpdateSegment(const int& i, const int& pre, const int& post)
{
	if (pre != post)
	{
		#pragma omp critical
		{
			if (segments.indices[i].next)
			{
				segments.indices[i].next->prev = segments.indices[i].prev;
			}
			if (segments.indices[i].prev)
			{
				segments.indices[i].prev->next = segments.indices[i].next;
			}
			else segments.segments[pre] = segments.indices[i].next;

			segments.indices[i].next = segments.segments[post];
			segments.indices[i].prev = nullptr;

			if (segments.segments[post])
			{
				segments.segments[post]->prev = segments.indices + i;
			}
			segments.segments[post] = segments.indices + i;
		}
	}
}
void sort()
{
	const int power = powerof2(PARTICLES_NUMBER);
	int index = 0;

	for (int i = 1; i <= power; ++i)
	{
		for (int j = i; j > 0; --j)
		{
			int distance = pow(2, j);
			int n_comps = PARTICLES_NUMBER / distance;
			const int chunk = Max(PARTICLES_NUMBER / (threads * n_comps), 1);

			#pragma omp parallel for firstprivate(distance, j, i) schedule(static, chunk)
			for (int k = 0; k < n_comps; ++k)
			{
				for (int l = 0; l < distance / 2; ++l)
				{
					int ind1, ind2;
					ind1 = k * distance + l;

					if (j != i) ind2 = k * distance + l + distance / 2;
					else  ind2 = (k + 1) * distance - l - 1;

					unsigned segInd1 = GetSegmentIndex(particles.centers[ind1]);
					unsigned segInd2 = GetSegmentIndex(particles.centers[ind2]);

					if (segInd1 > segInd2)
					{
						std::swap(particles.centers[ind1], particles.centers[ind2]);
						std::swap(particles.dir[ind1], particles.dir[ind2]);
					}
				}
			}
			index += n_comps * (distance / 2);
		}
	}
}
void initParticles()
{
	srand(time(NULL));

	static constexpr int internal_margin = 2;
	//DBG::print(cellsSize, "segment number ");
	for (int i = 0; i < PARTICLES_NUMBER; ++i)
	{
		float x = (BOXMARGINX + internal_margin + rand() % (BOXWIDTH - 2 * internal_margin)) * coeff;
		float y = (BOXMARGINY + internal_margin + rand() % (BOXHEIGHT - 2 * internal_margin)) * coeff;

		particles.centers[i] = { x, y };
		prediction[i] = { x, y };

		external[i] = { 0.0, 0.0 };
	}

	//sort();
	distribute();
}
void distribute()
{
	for (int i = 0; i < PARTICLES_NUMBER; ++i)
	{
		unsigned unit = GetSegmentIndex(particles.centers[i]);
		segments.indices[i].next = segments.segments[unit];
		if (segments.segments[unit] != nullptr)
		{
			segments.segments[unit]->prev = &segments.indices[i];
		}
		segments.segments[unit] = &segments.indices[i];

		segments.segments[unit]->value = i;
	}
}

void collisionResponse(const vec2& pos, const int& Index)
{
	vec2 relative_pos = pos;

	relative_pos.x -= BOXMARGINX;
	relative_pos.y -= BOXMARGINY;

	if (relative_pos.x <= 0.0)
	{
		external[Index].x += -collision_penalty * relative_pos.x;
	}
	else if (relative_pos.x >= BOXWIDTH)
	{
		external[Index].x += -collision_penalty * (relative_pos.x - BOXWIDTH);
	}

	if (relative_pos.y <= 0.0)
	{
		external[Index].y += -collision_penalty * relative_pos.y;
	}
	else if (relative_pos.y >= BOXHEIGHT)
	{
		external[Index].y += -collision_penalty * (relative_pos.y - BOXHEIGHT);
	}
}
void boundaryCondition(const int& Index, vec2& dp)
{
	vec2 relative_pos = prediction[Index] + dp;

	relative_pos.x -= BOXMARGINX;
	relative_pos.y -= BOXMARGINY;

	if (relative_pos.x <= 0.0)
	{
		dp.x = 0.0;
		prediction[Index].x = BOXMARGINX;
	}
	else if (relative_pos.x >= BOXWIDTH)
	{
		dp.x = 0.0;
		prediction[Index].x = BOXMARGINX + BOXWIDTH;
	}

	if (relative_pos.y <= 0.0)
	{
		dp.y = 0.0;
		prediction[Index].y = BOXMARGINY;
	}
	else if (relative_pos.y >= BOXHEIGHT)
	{
		dp.y = 0.0;
		prediction[Index].y = BOXMARGINY + BOXHEIGHT;
	}
}
void collisionHandler(const int& Index, vec2& dp)
{
	collisionResponse(prediction[Index] + dp, Index);
	boundaryCondition(Index, dp);
}

inline vec2 calcGradient(const int& Index, const int& k)
{
	const vec2 shift(epsilon, epsilon);

	const vec2 vector = prediction[Index] - prediction[k];
	return -KernelVersion_1::calcPoly6Gradient(vector - shift);
}
inline scalar calcGradientLength2(const int& Index, const int& k)
{
	const vec2 shift(epsilon, epsilon);

	const vec2 vector = prediction[Index] - prediction[k];
	const scalar dst = glm::length(vector - shift);

	const scalar x = KernelVersion_1::calcPoly6DerivativeX(dst, vector.x - shift.x);
	const scalar y = KernelVersion_1::calcPoly6DerivativeY(dst, vector.y - shift.y);

	return x * x + y * y;
}

void calcLambda(const int& Index)
{
	const vec2 shift(epsilon, epsilon);

	alignas(64) float density = 0.0;
	alignas(64) float bottom = relaxation;

	const unsigned segInd = GetSegmentIndex(prediction[Index]);

	// #pragma omp parallel for firstprivate(segInd, mass, influenceRadius) reduction(+:density) reduction(+:bottom) schedule(static) num_threads(1)
	for (int locShift = 0; locShift < 9; locShift++)
	{
		const int location = GetLocationFromShift(segInd, locShift);
		if (location < 0) continue;

		for (Node* seg = segments.segments[location]; seg; seg = seg->next)
		{
			if (Index == seg->value) continue;

			const vec2 vector = prediction[Index] - prediction[seg->value];
			const scalar dst = glm::length(vector + shift);

			const scalar x = KernelVersion_1::calcPoly6DerivativeX(dst, vector.x + shift.x);
			const scalar y = KernelVersion_1::calcPoly6DerivativeY(dst, vector.y + shift.y);

			// calculate density
			density += mass * KernelVersion_1::calcPoly6(glm::length(vector));
			bottom += calcGradientLength2(Index, seg->value) + x * x + y * y;
		}
	}
	lambdas[Index] = -(density / targetDensity - 1.0f) / (bottom / targetDensity);
}

vec2 calcDeltaPosition(const int& Index)
{
	static const float vfp = KernelVersion_1::calcPoly6(delta_q);

	alignas(64) float dx = 0.0;
	alignas(64) float dy = 0.0;

	unsigned segInd = GetSegmentIndex(prediction[Index]);

	// #pragma omp parallel for firstprivate(segInd, tensible_instability_k, tensible_instability_n, vfp) reduction(+:dx) reduction(+:dy) schedule(dynamic) num_threads(9)
	for (int locShift = 0; locShift < 9; locShift++)
	{
		int location = GetLocationFromShift(segInd, locShift);
		if (location < 0) continue;

		for (Node* seg = segments.segments[location]; seg; seg = seg->next)
		{
			if (seg->value == Index) continue;

			vec2 dir = prediction[Index] - prediction[seg->value];
			scalar dst = glm::length(dir);
			scalar left, s_corr;

			left = lambdas[Index] + lambdas[seg->value];

			s_corr = KernelVersion_1::calcPoly6(dst);
			s_corr /= vfp;

			s_corr *= s_corr;
			s_corr *= s_corr;
			s_corr *= -tensible_instability_k;

			dx += (left + s_corr) * KernelVersion_1::calcPoly6DerivativeX(dst, dir.x);
			dy += (left + s_corr) * KernelVersion_1::calcPoly6DerivativeY(dst, dir.y);
		}
	}

	return vec2(dx, dy);
}
vec2 calcVorticityAndViscosity(const int& Index)
{
	alignas(64) float x = 0.0;
	alignas(64) float y = 0.0;

	unsigned segInd = GetSegmentIndex(prediction[Index]);

	//#pragma omp parallel for firstprivate(segInd, viscosity_c) reduction(+:x) reduction(+:y) schedule(static) num_threads(1)
	for (int locShift = 0; locShift < 9; locShift++)
	{
		int location = GetLocationFromShift(segInd, locShift);
		if (location < 0) continue;

		for (Node* seg = segments.segments[location]; seg; seg = seg->next)
		{
			if (seg->value == Index) continue;

			// vorticity confinement
			vec2 dir = prediction[Index] - prediction[seg->value];
			float dst = glm::length(dir);

			// viscosity
			float influence = KernelVersion_1::calcViscosityKernel(dst);

			x += -dir.x * influence * viscosity_c - KernelVersion_1::calcPoly6DerivativeY(dst, dir.y);;
			y += -dir.y * influence * viscosity_c + KernelVersion_1::calcPoly6DerivativeX(dst, dir.x);
		}
	}
	//std::cout << x << " " << y << std::endl;
	return vec2(x, y);
}
vec2 ExternalForces(const vec2& pos, const vec2& velocity)
{
	// Gravity
	vec2 gravityAccel(0, -gravity);

	// Input interactions modify gravity
	if (interactionInputStrength != 0) {
		vec2 inputPointOffset = interactionInputPoint - pos;
		float sqrDst = glm::dot(inputPointOffset, inputPointOffset);
		if (sqrDst < interactionInputRadius * interactionInputRadius)
		{
			float dst = sqrt(sqrDst);
			float edgeT = (dst / interactionInputRadius);
			float centreT = 1 - edgeT; 
			vec2 dirToCentre = inputPointOffset / dst;
			float gravityWeight = 1 - (centreT * glm::clamp(interactionInputStrength / 10.0, 0.0, 1.0));
			vec2 accel = gravityAccel * gravityWeight + dirToCentre * centreT * interactionInputStrength;
			accel -= velocity * centreT;
			return accel;
		}
	}

	return gravityAccel;
}
