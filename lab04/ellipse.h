#ifndef ELLIPSE_H
#define ELLIPSE_H

#include "canvas.h"

void canonical(const QPoint &center, int a, int b, Canvas &canvas);
void parametric(const QPoint &center, int a, int b, Canvas &canvas);
void bresenham(const QPoint &center, int a, int b, Canvas &canvas);
void midPoint(const QPoint &center, int a, int b, Canvas &canvas);
void defaultQt(const QPoint &center, int a, int b, Canvas &canvas);

#endif // ELLIPSE_H
