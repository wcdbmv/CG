#include "ellipse.h"
#include "draw4points.h"
#include <QPainter>
#include <cmath>

void canonical(const QPoint &c, int a, int b, Canvas &canvas)
{
	for (int x = 0; x <= a; ++x) {
		const int y = round(b * sqrt(1.0 - static_cast<double>(x*x) / (a*a)));
		draw4points(c, x, y, canvas);
	}

	for (int y = 0; y <= b; ++y) {
		const int x = round(a * sqrt(1.0 - static_cast<double>(y*y) / (b*b)));
		draw4points(c, x, y, canvas);
	}
}

void parametric(const QPoint &c, int a, int b, Canvas &canvas)
{
	const int m = qMax(a, b);
	const int length4 = round(M_PIl * m / 2.0);
	for (int i = 0; i <= length4; ++i) {
		const double t = static_cast<double>(i) / m;
		const int x = round(a * cos(t));
		const int y = round(b * sin(t));
		draw4points(c, x, y, canvas);
	}
}

template <typename T>
static inline T sqr(T x) { return x * x; }

void bresenham(const QPoint &c, int a, int b, Canvas &canvas)
{
	int x = 0;
	int y = b;
	const int a2 = sqr<int>(a);
	const int b2 = sqr<int>(b);
	int d = round((a2 + b2) / 2.0 - 2 * a2 * b);
	while (y >= 0) {
		draw4points(c, x, y, canvas);
		if (d < 0) { // пиксел лежит внутри эллипса
			const int buf = 2 * (d + a2 * y) - a2;
			++x;
			if (buf <= 0) // горизонтальный шаг
				d += b2 * (2 * x + 1);
			else { // диагональный шаг
				--y;
				d += 2 * (b2 * x - a2 * y) + a2 + b2;
			}
		}
		else if (d > 0) { // пиксел лежит вне эллипса
			const int buf = 2 * (d - b2 * x) - b2;
			--y;
			if (buf > 0) // вертикальный шаг
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

void midPoint(const QPoint &c, int a, int b, Canvas &canvas)
{
	int x = 0;
	int y = b;
	const int a2 = sqr<int>(a);
	const int b2 = sqr<int>(b);
	double p = b2 - a2 * b + 0.25 * a2; // начальное значение параметра принятия решения в области tg < 1
	while (b2 * x < a2 * y) { // пока тангенс угла наклона меньше 1
		draw4points(c, x, y, canvas);

		++x;
		if (p < 0) // средняя точка внутри эллипса, ближе верхний пиксел, горизонталный шаг
			p += b2 * (2 * x + 1);
		else { // средняя точка вне эллипса, ближе диагональный пиксел, диагональный шаг
			--y;
			p += b2 * (2 * x + 1) - 2 * a2 * y;
		}
	}

	p = b2 * sqr<int>(x + 0.5) + a2 * sqr<int>(y - 1) - a2 * b2; // начальное значение параметра принятия решения в области tg > 1 в точке (x+0.5, y-1) последнего положения
	while (y >= 0) {
		draw4points(c, x, y, canvas);

		--y;
		if (p > 0)
			p -= a2 * (1 + 2 * y);
		else {
			++x;
			p += a2 * (1 - 2 * y) + 2 * b2 * x;
		}
	}
}

void defaultQt(const QPoint &c, int a, int b, Canvas &canvas)
{
	QPixmap pixmap = QPixmap::fromImage(*canvas.image);
	QPainter painter(&pixmap);
	painter.setPen(*canvas.color);

	defaultQtCore(c, a, b, painter);

	painter.end();
	*canvas.image = pixmap.toImage();
}

void defaultQtCore(const QPoint &c, int a, int b, QPainter &painter)
{
	painter.drawEllipse(c.x() - a, c.y() - b, a * 2, b * 2);
}
