#ifndef LINE_H
#define LINE_H

#include "canvas.h"
#include <QPainter>

bool dda(const QLine &line, Canvas &canvas);
bool bresenhamFloat(const QLine &line, Canvas &canvas);
bool bresenhamInteger(const QLine &line, Canvas &canvas);
bool bresenhamAntialiased(const QLine &line, Canvas &canvas);
bool defaultQt(const QLine &line, Canvas &canvas);
bool defaultQtCore(const QLine &line, QPainter &painter);
bool wu(const QLine &line, Canvas &canvas);

#endif // LINE_H
