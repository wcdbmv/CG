#ifndef CIRCLE_H
#define CIRCLE_H

#include "canvas.h"

void canonical(const QPoint &center, int radius, Canvas &canvas);
void parametric(const QPoint &center, int radius, Canvas &canvas);
void bresenham(const QPoint &center, int radius, Canvas &canvas);
void midPoint(const QPoint &center, int radius, Canvas &canvas);
void defaultQt(const QPoint &center, int radius, Canvas &canvas);
void defaultQtCore(const QPoint &center, int radius, QPainter &painter);


#endif // CIRCLE_H
