#ifndef LINE_H
#define LINE_H

#include "canvas.h"
#include <QPainter>

void dda(const QLine &line, Canvas &canvas);
void bresenhamFloat(const QLine &line, Canvas &canvas);
void bresenhamInteger(const QLine &line, Canvas &canvas);
void bresenhamAntialiased(const QLine &line, Canvas &canvas);
void defaultQt(const QLine &line, Canvas &canvas);
void defaultQtCore(const QLine &line, QPainter &painter);
void wu(const QLine &line, Canvas &canvas);

#endif // LINE_H
