#include "transform.h"

#include <cstring>
#include <cmath>

const Transform Transform::identity = {
	1, 0, 0,
	0, 1, 0,
	0, 0, 1
};

Transform::Transform() {
	*this = identity;
}

Transform::Transform(const Transform& other) {
	std::memcpy(matrix, other.matrix, 9 * sizeof (double));
}

Transform::Transform(
	double m00, double m01, double m02,
	double m10, double m11, double m12,
	double m20, double m21, double m22
) {
	matrix[0] = m00; matrix[1] = m01; matrix[2] = m02;
	matrix[3] = m10; matrix[4] = m11; matrix[5] = m12;
	matrix[6] = m20; matrix[7] = m21; matrix[8] = m22;
}

void Transform::translate(const Point& offset) {
	Transform translation = {
		1, 0, offset.x,
		0, 1, offset.y,
		0, 0, 1
	};
	combine(translation);
}

void Transform::scale(const Point &factor, const Point &center) {
	Transform scaling = {
		factor.x, 0,        center.x * (1 - factor.x),
		0,        factor.y, center.y * (1 - factor.y),
		0,        0,        1
	};
	combine(scaling);
}

void Transform::rotate(double angle, const Point &center) {
	double cos_a = std::cos(angle);
	double sin_a = std::sin(angle);
	Transform rotation = {
		cos_a, -sin_a, center.x * (1 - cos_a) + center.y * sin_a,
		sin_a, cos_a,  center.y * (1 - cos_a) - center.x * sin_a,
		0,     0,      1
	};
	combine(rotation);
}

void Transform::combine(const Transform &other) {
	const double* a = matrix;
	const double* b = other.matrix;

	*this = Transform(
		a[0] * b[0] + a[1] * b[3] + a[2] * b[6],
		a[0] * b[1] + a[1] * b[4] + a[2] * b[7],
		a[0] * b[2] + a[1] * b[5] + a[2] * b[8],
		a[3] * b[0] + a[4] * b[3] + a[5] * b[6],
		a[3] * b[1] + a[4] * b[4] + a[5] * b[7],
		a[3] * b[2] + a[4] * b[5] + a[5] * b[8],
		a[6] * b[0] + a[7] * b[3] + a[8] * b[6],
		a[6] * b[1] + a[7] * b[4] + a[8] * b[7],
		a[6] * b[2] + a[7] * b[5] + a[8] * b[8]
	);
}

Point Transform::apply(const Point& point) const {
	return Point(
		matrix[0] * point.x + matrix[1] * point.y + matrix[2],
		matrix[3] * point.x + matrix[4] * point.y + matrix[5]
	);
}

