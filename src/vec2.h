// (c) 2021 Ruggero Rossi
// very simple 2D vector inline class for robosoc2d
#ifndef VEC2_H
#define VEC2_H

#ifdef _WIN32
#include  <numeric>
//#define __WXMSW__
//#define _UNICODE
//#define NDEBUG
#else // __linux__ 
#endif

#define _USE_MATH_DEFINES
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace r2s {

struct Vec2 {
	double x, y;
public:
	Vec2() : x(0), y(0) {}
	Vec2(double x, double y) : x(x), y(y) {}
	Vec2(const Vec2& v) : x(v.x), y(v.y) {}
	Vec2(double angle) : x(cos(angle)), y(sin(angle)) {}
	Vec2& operator=(const Vec2& v) { x = v.x; y = v.y; return *this; }
	void set(double vx, double vy) { x = vx; y = vy; }
	void zero() { x=0.0; y=0.0; }
	void invert() { x=-x; y=-y;}
	Vec2 operator+(const Vec2& v) { return Vec2(x + v.x, y + v.y); }
	Vec2 operator-(const Vec2& v) { return Vec2(x - v.x, y - v.y); }
	Vec2& operator+=(const Vec2& v) { x += v.x; y += v.y; return *this; }
	Vec2& operator-=(const Vec2& v) { x -= v.x; y -= v.y; return *this; }
	Vec2& add(double vx, double vy) { x += vx; y += vy; return *this; }
	Vec2& sub(double vx, double vy) { x -= vx; y -= vy; return *this; }
	Vec2 operator*(double k) { return Vec2(x * k, y * k); }
	Vec2 operator/(double k) { return Vec2(x / k, y / k); }
	Vec2& operator*=(double k) { x *= k; y *= k; return *this; }
	Vec2& operator/=(double k) { x /= k; y /= k; return *this; }
	double len() const { return sqrt(x * x + y * y); }

	void norm() {
		double l = len();
		if (l != 0.0) {
			double k = 1.0 / l;
			x *= k;
			y *= k;
		}
	}
	void resize(double length) { norm(); x *= length; y *= length; }
	double dist(const Vec2& v) const { Vec2 d(v.x - x, v.y - y); return d.len(); }

	void rot(double rad) {
		double c = cos(rad);
		double s = sin(rad);
		double rx = x * c - y * s;
		double ry = x * s + y * c;
		x = rx;
		y = ry;
	}

	void rotDeg(double deg) { double rad = deg / 180.0 * M_PI; rot(rad); }

	double dot(const Vec2& v) { return x * v.x + y * v.y; }
	double cosBetween(const Vec2& v) { double l= len()*v.len(); return (l==0.0 ? 0.0 : dot(v)/l); }	// cosine of the angle between the two vectors
};

} // end namespace
#endif // VEC2_H

