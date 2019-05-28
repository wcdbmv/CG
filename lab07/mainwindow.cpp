#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QColorDialog>
#include <QLayout>
#include <cmath>

const double EPS = sqrt(2);

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	pixmap = QPixmap(ui->drawLabel->width(), ui->drawLabel->height());
	ui->drawLabel->setPixmapPointer(pixmap);

	on_clearPushButton_clicked();
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::on_addLinePushButton_clicked()
{
	const int x1 = ui->x1addLineSpinBox->value();
	const int y1 = ui->y1addLineSpinBox->value();
	const int x2 = ui->x2addLineSpinBox->value();
	const int y2 = ui->y2addLineSpinBox->value();

	addLine(QLine(x1, y1, x2, y2));
}

void MainWindow::on_setClipperPushButton_clicked()
{
	int x1 = ui->x1setClipperSpinBox->value();
	int y1 = ui->y1setClipperSpinBox->value();
	int x2 = ui->x2setClipperSpinBox->value();
	int y2 = ui->y2setClipperSpinBox->value();

	if (x1 > x2)
		qSwap(x1, x2);

	if (y1 > y2)
		qSwap(y1, y2);


	setClipper(QRect(x1, y1, x2 - x1, y2 - y1));
}

void MainWindow::on_setLineBaseColorPushButton_clicked()
{
	setLineBaseColor(QColorDialog::getColor(lineBaseColor, this, "Pick a line base color", QColorDialog::DontUseNativeDialog));
	displayImage();
}

void MainWindow::on_setClipperColorPushButton_clicked()
{
	setClipperColor(QColorDialog::getColor(clipperColor, this, "Pick a clipper color", QColorDialog::DontUseNativeDialog));
	displayImage();
}

void MainWindow::on_setClippedLineColorPushButton_clicked()
{
	setClippedLineColor(QColorDialog::getColor(clippedLineColor, this, "Pick a clipped line color", QColorDialog::DontUseNativeDialog));
	displayImage();
}

void MainWindow::on_clipPushButton_clicked()
{
	QPainter painter(&pixmap);
	painter.setPen(QPen(clippedLineColor, 3));

	const int xl = clipper.x();
	const int xr = xl + clipper.width();
	const int yb = clipper.y();
	const int yt = yb + clipper.height();

	for (auto &line: lines)
		clipLine(line, xl, xr, yb, yt, painter);

	ui->drawLabel->update();
}

void MainWindow::on_clearPushButton_clicked()
{
	lines.clear();
	clipper = QRect(0, 0, 1, 1);
	displayImage();

	setLineBaseColor(Qt::red);
	setClipperColor(Qt::black);
	setClippedLineColor(Qt::blue);

	left_clicked = false;
	right_clicked = false;
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
	int x = event->x() - ui->drawLabel->x();
	int y = event->y() - ui->drawLabel->y();

	if (x < 0 || x >= ui->drawLabel->width() || y < 0 || y >= ui->drawLabel->height())
		return;

	switch (event->button()) {
	case Qt::LeftButton:
		if (left_clicked) {
			receiveKeyboardModifiers(x, y);
			ui->x2addLineSpinBox->setValue(x);
			ui->y2addLineSpinBox->setValue(y);
			on_addLinePushButton_clicked();
		}
		else {
			ui->x1addLineSpinBox->setValue(x);
			ui->y1addLineSpinBox->setValue(y);
			left_clicked = true;
		}
		break;
	case Qt::RightButton:
		if (right_clicked) {
			ui->x2setClipperSpinBox->setValue(x);
			ui->y2setClipperSpinBox->setValue(y);
			on_setClipperPushButton_clicked();
		}
		else {
			ui->x1setClipperSpinBox->setValue(x);
			ui->y1setClipperSpinBox->setValue(y);
			right_clicked = true;
		}
	default:
		break;
	}
}

void MainWindow::receiveKeyboardModifiers(int &x, int &y)
{
	const int x1 = ui->x1addLineSpinBox->value();
	const int y1 = ui->y1addLineSpinBox->value();

	switch (QApplication::keyboardModifiers()) {
	case Qt::ShiftModifier: {
		y = y1;
		break;
	}
	case Qt::ControlModifier: {
		x = x1;
		break;
	}
	case Qt::ShiftModifier | Qt::ControlModifier: {
		const int dx = x - x1;
		const int dy = y - y1;
		if (dx * dy >= 0) {
			const int d = (dx + dy) / 2;
			x = x1 + d;
			y = y1 + d;
		}
		else {
			const int d = (dx - dy) / 2;
			x = x1 + d;
			y = y1 - d;
		}
		break;
	}
	default:
		break;
	}
}

void MainWindow::displayImage()
{
	pixmap.fill();

	QPainter painter(&pixmap);

	painter.setPen(QPen(clipperColor, 3));
	painter.drawRect(clipper);

	painter.setPen(lineBaseColor);
	for (auto const &line: lines)
		painter.drawLine(line);

	ui->drawLabel->update();
}

void MainWindow::setLineBaseColor(const QColor &color)
{
	lineBaseColor = color;
	colorLabel(lineBaseColor, ui->lineBaseColorLabel);
}

void MainWindow::setClipperColor(const QColor &color)
{
	clipperColor = color;
	colorLabel(clipperColor, ui->clipperColorLabel);
}

void MainWindow::setClippedLineColor(const QColor &color)
{
	clippedLineColor = color;
	colorLabel(clippedLineColor, ui->clippedLineColorLabel);
}

void MainWindow::colorLabel(const QColor &color, QLabel *label) {
	QPalette palette = label->palette();
	palette.setColor(label->backgroundRole(), color);
	label->setAutoFillBackground(true);
	label->setPalette(palette);
}

void MainWindow::addLine(const QLine &line)
{
	lines.push_back(line);
	left_clicked = false;
	displayImage();
}

void MainWindow::setClipper(const QRect &clipper)
{
	this->clipper = clipper;
	right_clicked = false;
	displayImage();
}

void MainWindow::clipLine(const QLine &line, int xl, int xr, int yb, int yt, QPainter &painter)
{
	QPoint p1 = line.p1();
	QPoint p2 = line.p2();
	for (int i = 1; i <= 3; ++i) {
		int t1 = code(p1, xl, xr, yb, yt);
		int t2 = code(p2, xl, xr, yb, yt);

		if (!t1 && !t2) { // line is fully visible
			painter.drawLine(p1, p2);
			return;
		}

		if (t1 & t2) // line is fully invisible
			return;

		QPoint r = p1;
		if (i > 2) {
			if ((t1 & t2) == 0)
				painter.drawLine(p1, p2);
			return;
		}

		if (t2) {
			// TODO: сделать нормальный выход из цикла
			while (qAbs(p1.x() - p2.x()) > EPS || qAbs(p1.y() - p2.y()) > EPS) {
				const QPoint pm = (p1 + p2) / 2;
				const int tm = code(pm, xl, xr, yb, yt);
				if (tm & t2)
					p2 = pm;
				else
					p1 = pm;
			}
		}
		p1 = p2;
		p2 = r;
	}
}

int MainWindow::code(const QPoint& point, int xl, int xr, int yb, int yt)
{
	int result = 0;

	if (point.x() < xl)
		result |= 1;
	if (point.x() > xr)
		result |= 1 << 1;
	if (point.y() < yb)
		result |= 1 << 2;
	if (point.y() > yt)
		result |= 1 << 3;

	return result;
}

