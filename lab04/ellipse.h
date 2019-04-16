#ifndef ELLIPSE_H
#define ELLIPSE_H

#include "canvas.h"

void canonical(const QPoint &center, const int a, const int b, Canvas &canvas);
void parametric(const QPoint &center, const int a, const int b, Canvas &canvas);
void bresenham(const QPoint &center, const int a, const int b, Canvas &canvas);
void midPoint(const QPoint &center, const int a, const int b, Canvas &canvas);
void defaultQt(const QPoint &center, const int a, const int b, Canvas &canvas);
void defaultQtCore(const QPoint &center, const int a, const int b, QPainter &painter);

#endif // ELLIPSE_H
