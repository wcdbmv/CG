#include "draw4points.h"

void draw4points(const QPoint &c, int x, int y, Canvas &canvas) {
	canvas.image->setPixel(c.x() + x, c.y() + y, canvas.color->rgb());
	canvas.image->setPixel(c.x() + x, c.y() - y, canvas.color->rgb());
	canvas.image->setPixel(c.x() - x, c.y() + y, canvas.color->rgb());
	canvas.image->setPixel(c.x() - x, c.y() - y, canvas.color->rgb());
}
