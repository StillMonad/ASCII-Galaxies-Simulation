#pragma once
#include <cmath>

struct vec2
{
	float x, y;

	vec2()
	{
		x = 0.0f;
		y = 0.0f;
	}

	vec2(float x, float y)
		:x(x), y(y)
	{}

	vec2(float k)
	{
		x = k;
		y = k;
	}

	vec2 operator + (const vec2& v) const
	{
		return vec2
		(
			x + v.x,
			y + v.y
		);
	}

	vec2 operator - (const vec2& v) const
	{
		return vec2
		(
			x - v.x,
			y - v.y
		);
	}

	float operator * (const vec2& v) const
	{
		return x * v.x + y * v.y;
	}

	vec2 operator * (float k) const
	{
		return vec2
		(
			k * x,
			k * y
		);
	}
	vec2 operator / (float k) const
	{
		return vec2
		(
			x / k,
			y / k
		);
	}

	void operator += (const vec2& v)
	{
		x += v.x;
		y += v.y;
	}

	void operator -= (const vec2& v)
	{
		x -= v.x;
		y -= v.y;
	}

	void operator *= (float k)
	{
		x *= k;
		y *= k;
	}

	void operator /= (float k)
	{
		x /= k;
		y /= k;
	}

	vec2 operator - ()
	{
		return vec2(-x, -y);
	}

	float magnitude2() const
	{
		return x * x + y * y;
	}

	float magnitude() const
	{
		return sqrt(magnitude2());
	}

	void normalize()
	{
		float mag = magnitude();
		if (mag == 0.0f)
			return;
		(*this) /= mag;
	}

	float getAng(const vec2& a) {
		float c = *this * a;
		return acos(c/(this->magnitude() * a.magnitude()));
	}

	float getAngToX() {
		float ang = this->getAng({ 1, 0 });
		if (this->y < 0) return ang;
		else return M_PI + (M_PI - ang);
	}

	vec2 rotate(const float& rad) {
		float mag = this->magnitude();
		float ang = getAngToX() + rad + M_PI/2;
		vec2 ret = vec2(sin(ang), cos(ang)) * mag;
		return ret;
	}
};

vec2 operator * (float k, const vec2& v)
{
	return v * k;
}