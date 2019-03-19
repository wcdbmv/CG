#include "line.h"
#include <cmath>

template <typename T> int sgn1(T val) {
	return (val > 0) - (val < 0);
}

#define check_curr_point do { ok |= x == line.p2().x() && y == line.p2().y(); } while(0)

// Процедура разложения в растр отрезка по методу цифрового дифференциального анализатора (ЦДА)
bool dda(const QLine &line, Canvas &canvas)
{
	bool ok = false;

	const int deltaX = line.p2().x() - line.p1().x();
	const int deltaY = line.p2().y() - line.p1().y();

	int length = qMax(qAbs(deltaX), qAbs(deltaY));

	// Предполагается, что концы отрезка не совпадают
	if (!length) {
		canvas.image->setPixel(line.p1().x(), line.p1().y(), canvas.color->rgb());
		return true;
	}

	// Полагаем большее из приращений dx или dy равным единице растра
	const double dx = (double) deltaX / length;
	const double dy = (double) deltaY / length;

	// Округляем величины, а не отбрасываем дробную часть
	// Использование знаковой функции делает алгоритм пригодным для всех квадрантов
	double xf = line.p1().x() + 0.5 * sgn1(dx);
	double yf = line.p1().y() + 0.5 * sgn1(dy);

	// Начало основного цикла
	while (length--) {
		const int x = qRound(xf);
		const int y = qRound(yf);
		check_curr_point;
		canvas.image->setPixel(x, y, canvas.color->rgb());
		xf += dx;
		yf += dy;
	}

	return ok;
}
// Анализ отрезков, проведенных из точки (0, 0) в точку (—8, 4) и (8, —4), показывает, что
// разложенный в растр отрезок лежит по одну сторону от реального и что на одном из концов отрезка
// появляется лишняя точка, т. е. результат работы алгоритма зависит от ориентации. Следовательно,
// точность в концевых точках ухудшается. Далее, если вместо взятия целой части использовать
// округление до ближайшего целого, то результаты снова получатся разными. Таким образом, либо
// нужно использовать более сложный и более медленный алгоритм, либо надо отступиться от требования
// максимально точной аппроксимации. Вдобавок предложенный алгоритм имеет тот недостаток, что он
// использует вещественную арифметику.

// Алгоритм Брезенхема разложения в растр отрезка
//
// Алгоритм выбирает оптимальные растровые координаты для представления отрезка. В процессе работы
// одна из координат — либо x, либо y (в зависимости от углового коэффициента) — изменяется на
// единицу. Изменение другой координаты (либо на нуль, либо на единицу) зависит от расстояния между
// действительным положением отрезка и ближайшими координатами сетки. Такое расстояние мы назовем
// ошибкой.
bool bresenhamFloat(const QLine &line, Canvas &canvas)
{
	bool ok = false;

	// Предполагается, что концы отрезка не совпадают
	if (line.p1() == line.p2()) {
		canvas.image->setPixel(line.p1().x(), line.p1().y(), canvas.color->rgb());
		return true;
	}

	// Инициализация переменных
	double x = line.p1().x();
	double y = line.p1().y();
	int dx = line.p2().x() - line.p1().x();
	int dy = line.p2().y() - line.p1().y();
	const int sx = sgn1(dx);
	const int sy = sgn1(dy);
	dx = qAbs(dx);
	dy = qAbs(dy);

	// обмен значений dx и dy в зависимости от углового коэффициента наклона отрезка
	const bool swapped = dy > dx;
	if (swapped)
		qSwap(dx, dy);

	const double m = static_cast<double>(dy) / dx;

	// Инициализация e с поправкой на половину пиксела
	double e = m - 0.5;

	// Начало основного цикла
	for (int i = 0; i != dx + 1; ++i) {
		check_curr_point;
		canvas.image->setPixel(x, y, canvas.color->rgb());
		if (e >= 0.0) {
			if (swapped)
				x += sx;
			else
				y += sy;
			--e;
		}
		if (e < 0) {
			if (swapped)
				y += sy;
			else
				x += sx;
			e += m;
		}
	}

	return ok;
}

// Целочисленный алгоритм Брезенхема
bool bresenhamInteger(const QLine &line, Canvas &canvas)
{
	bool ok = false;

	// Предполагается, что концы отрезка не совпадают
	if (line.p1() == line.p2()) {
		canvas.image->setPixel(line.p1().x(), line.p1().y(), canvas.color->rgb());
		return true;
	}

	// Инициализация переменных
	int x = line.p1().x();
	int y = line.p1().y();
	int dx = line.p2().x() - line.p1().x();
	int dy = line.p2().y() - line.p1().y();
	const int sx = sgn1(dx);
	const int sy = sgn1(dy);
	dx = qAbs(dx);
	dy = qAbs(dy);

	// обмен значений dx и dy в зависимости от углового коэффициента наклона отрезка
	const bool swapped = dy > dx;
	if (swapped)
		qSwap(dx, dy);

	const int dx2 = 2 * dx;
	const int dy2 = 2 * dy;

	// Инициализация e с поправкой на половину пиксела
	int e = dy2 - dx;

	// Начало основного цикла
	for (int i = 0; i != dx + 1; ++i) {
		check_curr_point;
		canvas.image->setPixel(x, y, canvas.color->rgb());
		if (e >= 0.0) {
			if (swapped)
				x += sx;
			else
				y += sy;
			e -= dx2;
		}
		if (e < 0) {
			if (swapped)
				y += sy;
			else
				x += sx;
			e += dy2;
		}
	}

	return ok;
}

bool bresenhamAntialiased(const QLine &line, Canvas &canvas)
{
	bool ok = false;

	if (line.p1() == line.p2()) {
		canvas.image->setPixel(line.p1().x(), line.p1().y(), canvas.color->rgb());
		return true;
	}

	const int i_max = 255;
	int dx = line.p2().x() - line.p1().x();
	int dy = line.p2().y() - line.p1().y();
	int sx = sgn1(dx);
	int sy = sgn1(dy);
	dx = qAbs(dx);
	dy = qAbs(dy);
	double x = line.p1().x();
	double y = line.p1().y();

	const bool swapped = dy > dx;
	if (swapped)
		qSwap(dx, dy);

	double m = 0;
	if (dy)
		m = static_cast<double>(i_max * dy) / dx;

	// В качестве ошибки в данном алгоритме принимается часть площади пиксела, находящаяся под отрезком
	// Все величины умножены на максимальный уровень интенсивности
	double e = i_max / 2.0;
	double w = i_max - m;
	QColor color(canvas.color->rgba());
	for (int i = 0; i != dx + 1; ++i) {
		check_curr_point;
		color.setAlpha(i_max - e);
		canvas.image->setPixel(x, y, color.rgba());
		if (e <= w) {
			if (swapped)
				y += sy;
			else
				x += sx;
			// Если ордината соседнего пиксела не увеличивается, то площадь, находящаяся под отрезком, увеличивается на величину
			// площади прямоугольника со сторонами 1 и m, то есть e = e + m
			e += m;
		}
		// Если же ордината соседнего пиксела увеличивается на единицу, то вычисленная доля площади пиксела будет содержать
		// и площадь пиксела, через который отрезок не проходит, следовательно, необходимо вычесть величину площади пиксела
		// Поскольку доля площади не может быть отрицательной величиной, то по сравнению с ранее рассмотренными алгоритмами
		// Брезенхема необходимо скорректировать величину ошибки, прибавив к ней величину w = 1 - m.
		else {
			x += sx;
			y += sy;
			e -= w;
		}
	}

	return ok;
}

bool defaultQt(const QLine &line, Canvas &canvas)
{
	QPixmap pixmap = QPixmap::fromImage(*canvas.image);
	QPainter painter(&pixmap);
	painter.setPen(*canvas.color);

	defaultQtCore(line, painter);

	painter.end();
	*canvas.image = pixmap.toImage();
	*canvas.image = canvas.image->convertToFormat(QImage::Format_ARGB32);

	return true;
}

bool defaultQtCore(const QLine &line, QPainter &painter)
{
	painter.drawLine(line);
	return true;
}

static inline int ipart(double x) { return floor(x); }
static inline double fpart(double x) { return x - floor(x); }
static inline double rfpart(double x) { return 1 - fpart(x); }

bool wu(const QLine &line, Canvas &canvas)
{
	if (line.p1() == line.p2()) {
		canvas.image->setPixel(line.p1().x(), line.p1().y(), canvas.color->rgb());
		return true;
	}

	int x1 = line.p1().x();
	int y1 = line.p1().y();
	int x2 = line.p2().x();
	int y2 = line.p2().y();

	int dx = x2 - x1;
	int dy = y2 - y1;

	const bool swapped = qAbs(dx) < qAbs(dy);
	if (swapped) {
		qSwap(x1, y1);
		qSwap(x2, y2);
		qSwap(dx, dy);
	}
	if (x2 < x1) {
		qSwap(x1, x2);
		qSwap(y1, y2);
	}

	dx = x2 - x1;
	dy = y2 - y1;
	double grad = dx ? static_cast<double>(dy) / dx : 1;

	QColor color(canvas.color->rgba());

	double y = y1;
	for (int x = x1; x <= x2; ++x) {
		const int s = sgn1(y);
		if (swapped) {
			color.setAlphaF(rfpart(y));
			canvas.image->setPixel(ipart(y), x, color.rgba());
			color.setAlphaF(fpart(y));
			canvas.image->setPixel(ipart(y) + s, x, color.rgba());
		}
		else {
			color.setAlphaF(rfpart(y));
			canvas.image->setPixel(x, ipart(y), color.rgba());
			color.setAlphaF(fpart(y));
			canvas.image->setPixel(x, ipart(y) + s, color.rgba());
		}
		y += grad;
	}

	return true;
}
