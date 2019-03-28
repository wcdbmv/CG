#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "point.h"

class Transform {
public:
	static const Transform identity;

private:
	double matrix[9];

public:
	Transform();
	Transform(const Transform& other);
	Transform(
		double m00, double m01, double m02,
		double m10, double m11, double m12,
		double m20, double m21, double m22
	);

	void translate(const Point& offset);
	void scale(const Point& factor, const Point& center = Point());
	void rotate(double angle, const Point& center = Point());
	void combine(const Transform& other);

	Point apply(const Point& point) const;
};

#endif // TRANSFORM_H
