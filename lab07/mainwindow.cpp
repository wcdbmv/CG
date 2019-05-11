#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QColorDialog>
#include <QLayout>

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
	painter.setPen(clippedLineColor);

	const int xl = clipper.x();
	const int xr = xl + clipper.width();
	const int yt = clipper.y();
	const int yb = yt + clipper.height();

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

	painter.setPen(clipperColor);
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

void MainWindow::clipLine(QLine &line, int xl, int xr, int yb, int yt, QPainter &painter)
{
	QPoint p1 = line.p1();
	QPoint p2 = line.p2();

	int t1 = code(p1, xl, xr, yb, yt);
	int t2 = code(p2, xl, xr, yb, yt);

	QPoint p1_res = p1;
	QPoint p2_res = p2;

	if (!t1 && !t2) { // line is fully visible
		painter.drawLine(line);
		return;
	}

	if (t1 & t2)
		return; // line is fully invisible

	if (t1) // first point is inside the window
		p1_res = intersection(line, xl, xr, yb, yt, true);

	if (t2) // second point is inside the window
		p2_res = intersection(line, xl, xr, yb, yt, false);

	if (p1_res.x() == -1 || p2_res.x() == -1)
		return;

	painter.drawLine(p1_res, p2_res);
}

int MainWindow::code(const QPoint &p, int xl, int xr, int yb, int yt)
{
	int code = 0;

	if (p.x() < xl)
		code |= 1 << 3;
	if (p.x() > xr)
		code |= 1 << 2;
	if (p.y() > yb)
		code |= 1 << 1;
	if (p.y() < yt)
		code |= 1;

	return code;
}

QPoint MainWindow::intersection(QLine &line, int xl, int xr, int yb, int yt, bool first)
{
	QPoint p1 = line.p1();
	QPoint p2 = line.p2();

	double m = 1e10; // just very big value

	QPoint p = first? line.p1() : line.p2();

	if (p2.x() - p1.x() != 0) { // line isn't vertical
		// looking for intersection with left side
		m = double(p2.y() - p1.y()) / (p2.x() - p1.x());
		if (xl >= p.x()) {
			int y = qRound(m * (xl - p.x()) + p.y());
			if (y >= yt && y <= yb) {
				p.setY(y);
				p.setX(xl);
				return p; // intersection was found
			}
		}
		// loking for intersection with right side
		if (xr <= p.x()){
			int y = qRound(m * (xr - p.x()) + p.y());
			if (y >= yt && y <= yb) {
				p.setX(xr);
				p.setY(y);
				return p; // intersection was found
			}
		}
	}
	// check if line is horisontal
	if (m == 0.) {
		return QPoint(-1, -1);
	}
	// looking for intersection with top side
	if (yt >= p.y()) {
		int x = qRound((1 / m) * (yt - p.y()) + p.x());
		if (x >= xl && x <= xr) {
			p.setX(x);
			p.setY(yt);
			return p; // intersection was found
		}
	}
	if (yb > p.y()) {
		return QPoint(-1, -1); // error situation here, assert(false) can be used here
	}
	// looking for intersection with bottom side
	int x = qRound((1 / m) * (yb - p.y()) + p.x());
	if (x >= xl && x <= xr) {
		p.setX(x);
		p.setY(yb);
		return p; // intersection was found
	}
	// line is invisiable
	return QPoint(-1, -1);
}
