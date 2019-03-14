#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QColorDialog>
#include <QElapsedTimer>
#include <cmath>

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow),
	fgColor(defaultFgColor),
	image(QImage(721, 721, QImage::Format_ARGB32)),
	scene(new QGraphicsScene(0, 0, 720, 720))
{
	ui->setupUi(this);

	ui->graphicsView->setScene(scene);
	imageView();

	on_clearAllPushButton_clicked();
	on_setDefaultFGColorPushButton_clicked();
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::colorLabel(QLabel *label, QColor &color)
{
	QPalette palette = label->palette();
	palette.setColor(label->backgroundRole(), color);
	label->setAutoFillBackground(true);
	label->setPalette(palette);
}

void MainWindow::imageView() {
	scene->addPixmap(QPixmap::fromImage(image));
}

template <typename T> int sgn1(T val) {
	return (val > 0) - (val < 0);
}

// Процедура разложения в растр отрезка по методу цифрового дифференциального анализатора (ЦДА)
void MainWindow::dda(const QLine &line)
{
	const int deltaX = line.p2().x() - line.p1().x();
	const int deltaY = line.p2().y() - line.p1().y();

	int length = qMax(qAbs(deltaX), qAbs(deltaY));

	// Предполагается, что концы отрезка не совпадают
	if (!length) {
		image.setPixel(line.p1().x(), line.p1().y(), fgColor.rgb());
		return;
	}

	// Полагаем большее из приращений dx или dy равным единице растра
	const double dx = (double) deltaX / length;
	const double dy = (double) deltaY / length;

	// Округляем величины, а не отбрасываем дробную часть
	// Использование знаковой функции делает алгоритм пригодным для всех квадрантов
	double x = line.p1().x() + 0.5 * sgn1(dx);
	double y = line.p1().y() + 0.5 * sgn1(dy);

	// Начало основного цикла
	while (length--) {
		image.setPixel(qRound(x), qRound(y), fgColor.rgb());
		x += dx;
		y += dy;
	}
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
void MainWindow::bresenhamFloat(const QLine &line)
{
	// Предполагается, что концы отрезка не совпадают
	if (line.p1() == line.p2()) {
		image.setPixel(line.p1().x(), line.p1().y(), fgColor.rgb());
		return;
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
	for (int i = 0; i != dx; ++i) {
		image.setPixel(x, y, fgColor.rgb());
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
}

// Целочисленный алгоритм Брезенхема
void MainWindow::bresenhamInteger(const QLine &line)
{
	// Предполагается, что концы отрезка не совпадают
	if (line.p1() == line.p2()) {
		image.setPixel(line.p1().x(), line.p1().y(), fgColor.rgb());
		return;
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
	for (int i = 0; i != dx; ++i) {
		image.setPixel(x, y, fgColor.rgb());
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
}

void MainWindow::bresenhamAntialiased(const QLine &line)
{
	if (line.p1() == line.p2()) {
		image.setPixel(line.p1().x(), line.p1().y(), fgColor.rgb());
		return;
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
	double e = i_max / 2.0;
	double w = i_max - m;
	QColor color(fgColor.rgba());
	for (int i = 0; i != dx; ++i) {
		color.setAlpha(i_max - e);
		image.setPixel(x, y, color.rgba());
		if (e <= w) {
			if (swapped)
				y += sy;
			else
				x += sx;
			e += m;
		}
		else {
			x += sx;
			y += sy;
			e -= w;
		}
	}
}

void MainWindow::defaultQt(const QLine &line)
{
	QPixmap pixmap = QPixmap::fromImage(image);
	QPainter painter(&pixmap);
	painter.setPen(fgColor);

	painter.drawLine(line);

	painter.end();
	image = pixmap.toImage();
}

static inline int ipart(double x) { return floor(x); }
static inline double fpart(double x) { return x - floor(x); }
static inline double rfpart(double x) { return 1 - fpart(x); }

void MainWindow::wu(const QLine &line)
{
	if (line.p1() == line.p2()) {
		image.setPixel(line.p1().x(), line.p1().y(), fgColor.rgb());
		return;
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

	QColor color = fgColor;
	double y = y1;
	for (int x = x1; x <= x2; ++x) {
		const int s = sgn1(y);
		if (swapped) {
			color.setAlphaF(rfpart(y));
			image.setPixel(ipart(y), x, color.rgba());
			color.setAlphaF(fpart(y));
			image.setPixel(ipart(y) + s, x, color.rgba());
		}
		else {
			color.setAlphaF(rfpart(y));
			image.setPixel(x, ipart(y), color.rgba());
			color.setAlphaF(fpart(y));
			image.setPixel(x, ipart(y) + s, color.rgba());
		}
		y += grad;
	}
}
void MainWindow::on_fgPushButton_clicked()
{
	fgColor = QColorDialog::getColor(fgColor, this, "Pick a FG color", QColorDialog::DontUseNativeDialog);
	colorLabel(ui->fgLabel, fgColor);
}

void MainWindow::on_setBGColorToFGPushButton_clicked()
{
	fgColor = defaultBgColor;
	colorLabel(ui->fgLabel, fgColor);
}

void MainWindow::drawLine(const QLine &line) {
	if (ui->ddaRadioButton->isChecked())
		dda(line);
	else if (ui->bresenhamFloatRadioButton->isChecked())
		bresenhamFloat(line);
	else if (ui->bresenhamIntegerRadioButton->isChecked())
		bresenhamInteger(line);
	else if (ui->bresenhamAntialiasedRadioButton->isChecked())
		bresenhamAntialiased(line);
	else if (ui->defaultQtRadioButton->isChecked())
		defaultQt(line);
	else if (ui->wuRadioButton->isChecked())
		wu(line);
	else
		Q_ASSERT(0);
}

void MainWindow::on_drawLinePushButton_clicked()
{
	const int x1 = 360 + ui->x1SpinBox->text().toInt();
	const int y1 = 360 - ui->y1SpinBox->text().toInt();
	const int x2 = 360 + ui->x2SpinBox->text().toInt();
	const int y2 = 360 - ui->y2SpinBox->text().toInt();
	const QLine line(x1, y1, x2, y2);

	QElapsedTimer timer;
	timer.start();

	drawLine(line);

	ui->statusBar->showMessage(QString::number(timer.nsecsElapsed()));

	imageView();
}

static inline double toRadians(double x) { return x * M_PIl / 180; }

void MainWindow::on_drawSunPushButton_clicked()
{
	int length = ui->lengthSpinBox->text().toInt();
	int dangle = ui->angleSpinBox->text().toInt();
	for (int angle = 0; angle < 360; angle += dangle)
		drawLine(QLine(360, 360,
			360 + round(length * cos(toRadians(angle))),
			360 - round(length * sin(toRadians(angle)))));
	imageView();
}

void MainWindow::on_clearAllPushButton_clicked()
{
	ui->statusBar->showMessage("");
	image.fill(defaultBgColor);
	imageView();
}

void MainWindow::on_setDefaultFGColorPushButton_clicked()
{
	fgColor = defaultFgColor;
	colorLabel(ui->fgLabel, fgColor);
}
