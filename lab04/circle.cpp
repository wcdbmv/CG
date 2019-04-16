#include "circle.h"
#include "draw4points.h"
#include <QPainter>
#include <cmath>

const float EPS = 1e-5;

void canonical(const QPoint &c, const int r, Canvas &canvas)
{
	const int r2 = r*r;
	const int deltaX = qRound(r / sqrt(2));
	for (int x = 0; x <= deltaX; ++x) {
		const int y = qRound(sqrt(r2 - x*x));
		draw4points(c, x, y, canvas);
		draw4points(c, y, x, canvas);
	}
}

void parametric(const QPoint &c, const int r, Canvas &canvas)
{
	const float dt = 1.0f / r;
	for (float t = M_PI / 2.0f; t >= -dt / 2.0f; t -= dt) {
		const int x = qRound(r * cos(t));
		const int y = qRound(r * sin(t));
		draw4points(c, x, y, canvas);
		draw4points(c, y, x, canvas);
	}
}

void bresenham(const QPoint &c, const int r, Canvas &canvas)
{
	int x = 0;
	int y = r;

	// разность квадратов расстояний от центра окружности до диагонального пиксела и до идеальной окружности:
	// d = (x + 1)^2 + (y - 1)^2 - r^2 = 1 + (r - 1)^2 - r^2 = 2(1 - r)
	int d = 2 * (1 - r);
	while (y >= 0) {
		draw4points(c, x, y, canvas);

		if (d < 0) { // пиксел внутри окружности
			const int d1 = 2 * (d + y) - 1; // lг - lд
			++x;
			if (d1 <= 0) // горизонтальный шаг
				d += 2 * x + 1;
			else { // диагональный шаг
				--y;
				d += 2 * (x - y + 1);
			}
		}
		else if (d > 0) { // пиксел лежит вне окружности
			const int d2 = 2 * (d - x) - 1; // lв - lд
			--y;
			if (d2 > 0) // вертикальный шаг
				d += 1 - 2 * y;
			else { // диагональный шаг
				++x;
				d += 2 * (x - y + 1);
			}
		}
		else { // пиксель лежит на окружности
			++x;
			--y;
			d += 2 * (x - y + 1);
		}
	}
}

void midPoint(const QPoint &c, const int r, Canvas &canvas)
{
	int x = 0;
	int y = r;
	int d = 1 - r;
	do {
		draw4points(c, x, y, canvas);
		draw4points(c, y, x, canvas);

		++x;
		if (d < 0) // средняя точка внутри окружности, ближе верхний пиксел, горизонтальный шаг
			d += 2 * x + 1;
		else { // средняя точка вне окружности, ближе диагональный пиксел, диагональный шаг
			--y;
			d += 2 * (x - y) + 1;
		}
	} while (x <= y);
}

void defaultQt(const QPoint &c, const int r, Canvas &canvas)
{
	QPixmap pixmap = QPixmap::fromImage(*canvas.image);
	QPainter painter(&pixmap);
	painter.setPen(*canvas.color);

	defaultQtCore(c, r, painter);

	painter.end();
	*canvas.image = pixmap.toImage();
}

void defaultQtCore(const QPoint &c, const int r, QPainter &painter)
{
	const int _2r = r * 2;
	painter.drawEllipse(c.x() - r, c.y() - r, _2r, _2r);
}

