#ifndef KERNELS
#define KERNELS

#include "../settings.hpp"
#include <numbers>

namespace KernelVersion_1
{
	constexpr scalar r = influenceRadius;
	constexpr scalar r9 = r * r * r * r * r * r * r * r * r;

	inline scalar calcPoly6(scalar dst)
	{
		if (dst >= r || dst <= 0.0f) return 0.0f;
		const scalar v = r * r - dst * dst;
		return 315.0f * v*v*v / (64.0f * std::_Pi_val * r9);
	}
	inline scalar calcSpikyKernel(scalar dst)
	{
		if (dst > r || dst <= 0.0f) return 0.0f;
		return 15.0f * pow(r - dst, 3.0) / (std::_Pi_val * pow(r, 6.0f));
	}
	inline scalar calcViscosityKernel(scalar dst)
	{
		if (dst > r || dst <= 0.0f) return 0.0f;

		float ret = -pow(dst, 3.0) / (2 * pow(r, 3.0));
		ret += (dst * dst) / (r * r);
		ret += r / (2 * dst) - 1;

		return 15.0f * ret / (2.0f * std::_Pi_val * pow(r, 3.0));
	}
	inline scalar calcPoly6DerivativeX(const vec2& pos)
	{
		const float l = glm::length(pos);
		if (l >r|| l <= 0) return 0;
		const scalar v = r * r - l * l;

		return -945.0 * v * v * pos.x / (32.0 * std::_Pi_val * r9);
	}
	inline scalar calcPoly6DerivativeX(const scalar& d, const scalar& posx)
	{
		if (d > r || d <= 0) return 0;
		const scalar v = r * r - d * d;

		return -945.0 * v * v * posx / (32.0 * std::_Pi_val * r9);
	}
	inline scalar calcSpikyDerivativeX(const vec2& pos)
	{
		float l = glm::length(pos);
		if (l >r|| l <= 0) return 0;

		return -45.0 * pow(r - l, 2.0) * pos.x / (l * std::_Pi_val * pow(r, 6.0));
	}
	inline scalar calcPoly6DerivativeY(const vec2& pos)
	{
		const float l = glm::length(pos);
		if (l >r|| l <= 0) return 0;
		const scalar v = r * r - l*l;

		return -945.0 * v*v * pos.y / (32.0 * std::_Pi_val * r9);

	}
	inline scalar calcPoly6DerivativeY(const scalar& d, const scalar& posy)
	{
		if (d > r || d <= 0) return 0;
		const scalar v = r * r - d * d;

		return -945.0 * v * v * posy / (32.0 * std::_Pi_val * r9);

	}
	inline scalar calcSpikyDerivativeY(const vec2& pos)
	{
		float l = glm::length(pos);
		if (l > r || l <= 0) return 0;

		return -45.0 * pow(r - l, 2.0) * pos.y / (l * std::_Pi_val * pow(r, 6.0));
	}
	inline scalar calcPoly6GradientCoeff(const scalar& d)
	{
		if (d >= r || d <= 0) return 0.0;
		const scalar diff = r * r - d * d;
		const scalar v = -945.0 * diff * diff / (32.0 * std::_Pi_val * r9);
		return v;
	}
	inline vec2   calcPoly6Gradient(const vec2& pos)
	{
		const scalar l = glm::length(pos);
		if (l > r || l <= 0) return vec2(0.0, 0.0);
		const scalar diff = r * r - l * l;
		const scalar v = -945.0 * diff*diff / (32.0 * std::_Pi_val * r9);
		return v * pos;
	}
	inline vec2   calcSpikyGradient(const vec2& pos)
	{
		float l = glm::length(pos);
		if (l >r|| l <= 0) return vec2(0.0, 0.0);

		const scalar v = -45.0 * pow(r - l, 2.0) / (l * std::_Pi_val * pow(r, 6.0));
		return vec2(v * pos.x, v * pos.y);
	}
	inline vec2   calcPoly6Gradient(const scalar& d, const vec2& pos)
	{
		if (d >= r || d <= 0) return vec2(0.0, 0.0);
		const scalar diff = r * r - d * d;
		const scalar v = -945.0 * diff * diff / (32.0 * std::_Pi_val * r9);
		return v * pos;
	}

}


#endif