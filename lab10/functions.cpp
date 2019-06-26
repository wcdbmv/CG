#include "functions.h"

const QVector<Function> FUNCTIONS = {
	{"|cos(x) * sin(x)|", [](double x, double z) {
			return std::abs(std::cos(x) * std::sin(z));
	}},

	{"exp(sin(x) * cos(z))", [](double x, double z) {
		return std::exp(std::sin(x) * std::cos(z));
	}},

	{"x + z", [](double x, double z) {
		return x + z;
	}},

	{"exp(|sin(x + z) * cos(x + z)|)", [](double x, double z) {
		return std::exp(std::abs(0.5 * std::sin(2.0 * (x + z))));
	}}
};
