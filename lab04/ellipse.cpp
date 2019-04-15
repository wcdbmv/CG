#include "ellipse.h"
#include "draw4points.h"
#include <QPainter>
#include <cmath>

void canonical(const QPoint &c, const int a, const int b, Canvas &canvas)
{
	const int a2 = a * a;
	const int b2 = b * b;

	const float bDivA = static_cast<float>(b) / a;
	const int deltaX = qRound(a2 / sqrt(a2 + b2));
	for (int x = 0; x <= deltaX; ++x) {
		const int y = qRound(sqrt(static_cast<float>(a2 - x*x)) * bDivA);
		draw4points(c, x, y, canvas);
	}

	const float aDivB = static_cast<float>(a) / b;
	const int deltaY = qRound(b2 / sqrt(a2 + b2));
	for (int y = 0; y <= deltaY; ++y) {
		const int x = qRound(sqrt(static_cast<float>(b2 - y*y)) * aDivB);
		draw4points(c, x, y, canvas);
	}
}


void parametric(const QPoint &c, const int a, const int b, Canvas &canvas)
{
	const float dt = 1.0f / qMax(a, b);
	for (float t = M_PI / 2.0f; t >= -dt / 2.0f; t -= dt) {
		const int x = qRound(a * cos(t));
		const int y = qRound(b * sin(t));
		draw4points(c, x, y, canvas);
	}
}

void bresenham(const QPoint &c, const int a, const int b, Canvas &canvas)
{
	int x = 0;
	int y = b;

	const int a2 = a * a;
	const int b2 = b * b;

	// разность квадратов расстояний от центра окружности эллипса до диагонального пиксела и до идеального эллипса
	int d = a2 + b2 - 2 * a2 * y;
	while (y >= 0) {
		draw4points(c, x, y, canvas);
		if (d < 0) { // пиксел лежит внутри эллипса
			const int d1 = 2 * (d + a2 * y) - a2; // lг - lд
			++x;
			if (d1 < 0) // горизонтальный шаг
				d += b2 * (2 * x + 1);
			else { // диагональный шаг
				--y;
				d += 2 * (b2 * x - a2 * y) + a2 + b2;
			}
		}
		else if (d > 0) { // пиксел лежит вне эллипса
			const int d2 = 2 * (d - b2 * x) - b2; // lв - lд
			--y;
			if (d2 >= 0) // вертикальный шаг
				d += a2 * (1 - 2 * y);
			else { // диагональный шаг
				++x;
				d += 2 * (b2 * x - a2 * y) + a2 + b2;
			}
		}
		else { // пиксел лежит на окружности, диагональный шаг
			++x;
			--y;
			d += 2 * (b2 * x - a2 * y) + a2 + b2;
		}
	}
}

void midPoint(const QPoint& c, const int a, const int b, Canvas &canvas) {
	const int a2 = a * a;
	const int b2 = b * b;

	int x = 0;
	int y = b;

	int f = b2 + a2 * (y - 0.5f) * (y - 0.5) - static_cast<long long>(a2) * b2;
	const int deltaX = a2 / sqrt(b2 + a2);
	while (x <= deltaX) {
		draw4points(c, x, y, canvas);

		++x;
		if (f > 0) {
			--y;
			f += -2 * a2 * y; // f += dy;
		}
		f += b2 * (2 * x + 1); // f += df;
	}

	f += 0.75f * (a2 - b2) - (b2 * x + a2 * y);
	while (y >= 0){
		draw4points(c, x, y, canvas);

		--y;
		if (f < 0) {
			++x;
			f += 2 * b2 * x; // f += dx;
		}
		f += a2 * (1 - 2 * y); // f += df;
	}
}

void defaultQt(const QPoint &c, const int a, const int b, Canvas &canvas)
{
	QPixmap pixmap = QPixmap::fromImage(*canvas.image);
	QPainter painter(&pixmap);
	painter.setPen(*canvas.color);

	defaultQtCore(c, a, b, painter);

	painter.end();
	*canvas.image = pixmap.toImage();
}

void defaultQtCore(const QPoint &c, const int a, const int b, QPainter &painter)
{
	painter.drawEllipse(c.x() - a, c.y() - b, 2 * a, 2 * b);
}
