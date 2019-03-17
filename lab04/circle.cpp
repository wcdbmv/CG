#include "circle.h"
#include "draw4points.h"
#include <QPainter>
#include <cmath>

void canonical(const QPoint &c, int r, Canvas &canvas)
{
	for (int x = 0; x <= r; ++x) {
		const int y = round(sqrt(r*r - x*x));
		draw4points(c, x, y, canvas);
	}

	for (int y = 0; y <= r; ++y) {
		const int x = round(sqrt(r*r - y*y));
		draw4points(c, x, y, canvas);
	}
}

void parametric(const QPoint &c, int r, Canvas &canvas)
{
	int length4 = round(M_PIl * r / 2);
	for (int i = 0; i <= length4; ++i) {
		const double t = static_cast<double>(i) / r;
		const int x = round(r * cos(t));
		const int y = round(r * sin(t));
		draw4points(c, x, y, canvas);
	}
}

void bresenham(const QPoint &c, int r, Canvas &canvas)
{
	int x = 0;
	int y = r;
	int d = 2 * (1 - r);
	while (y >= 0) {
		draw4points(c, x, y, canvas);

		if (d < 0) { // пиксел внутри окружности
			const int buf = 2 * (d + y) - 1;
			++x;

			if (buf <= 0) // горизонтальный шаг
				d += 2 * x + 1;
			else { // диагональный шаг
				--y;
				d += 2 * (x - y + 1);
			}
		}
		else if (d > 0) { // пиксел лежит вне окружности
			const int buf = 2 * (d - x) - 1;
			--y;

			if (buf > 0) // вертикальный шаг
				d += 1 - 2 * y;
			else { // диагональны шаг
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

void midPoint(const QPoint &c, int r, Canvas &canvas)
{
	int x = 0;
	int y = r;
	double p = 1.25 - r; // (x+1)^2 + (y-0.5)^2 - r^2
	do {
		draw4points(c, x, y, canvas);
		draw4points(c, y, x, canvas);

		++x;
		if (p < 0) // средняя точка внутри окружности, ближе верхний пиксел, горизонтальный шаг
			p += 2 * x + 1;
		else { // средняя точка вне окружности, ближе диагональный пиксел, диагональный шаг
			p += 2 * x - 2 * y + 5;
			--y;
		}
	} while (x <= y);
}

void defaultQt(const QPoint &c, int r, Canvas &canvas)
{
	QPixmap pixmap = QPixmap::fromImage(*canvas.image);
	QPainter painter(&pixmap);
	painter.setPen(*canvas.color);

	{
		const int _2r = r << 1;
		painter.drawEllipse(c.x() - r, c.y() - r, _2r, _2r);
	}

	painter.end();
	*canvas.image = pixmap.toImage();
}
