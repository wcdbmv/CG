#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QColorDialog>
#include <QMessageBox>
#include <QPainter>
#include <QLabel>

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow),
	closed(false),
	defaultBoundColor(Qt::black),
	defaultFillColor(230, 212, 255),
	fillColor(defaultFillColor),
	pixmap(721, 721),
	image(721, 721, QImage::Format_RGB32),
	start_point(0)
{
	ui->setupUi(this);

	ui->drawLabel->setPixmapPointer(pixmap);

	clearImage();
	colorLabel();
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{

	const int x = event->x() - ui->drawLabel->x();
	const int y = event->y() - ui->drawLabel->y();

	if (x < 0 || y < 0)
		return;

	if (event->button() == Qt::RightButton) {
		ui->x0SpinBox->setValue(x);
		ui->y0SpinBox->setValue(y);
		return;
	}

	addPoint(QPoint(x, y), drawType());
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
	const int x = event->x() - ui->drawLabel->x();
	const int y = event->y() - ui->drawLabel->y();

	if (x < 0 || y < 0)
		return;

	addPoint(QPoint(x, y), drawType());
}

void MainWindow::on_addPointPushButton_clicked()
{
	const int x = ui->xSpinBox->text().toInt();
	const int y = ui->ySpinBox->text().toInt();
	addPoint(QPoint(x, y), DrawType::none);
}

void MainWindow::on_closePushButton_clicked()
{
	if (edges.size() <= 2) {
		QMessageBox::critical(this, "Error", "Need at least 2 edges");
		return;
	}

	if (closed) {
		QMessageBox::critical(this, "Error", "Already closed");
		return;
	}

	addEdge(QLine(points[points.size() - 1], points[start_point]));
	closed = true;
}

void MainWindow::on_fillPushButton_clicked()
{
	if (!closed) {
		QMessageBox::critical(this, "Error", "Close figure!");
		return;
	}

	const int delay_value = ui->delaySpinBox->value();

	QPainter painter(&pixmap);
	painter.setPen(fillColor);
	QStack<QPoint> stack;
	stack.push(QPoint(ui->x0SpinBox->value(), ui->y0SpinBox->value()));
	while (!stack.empty()) {
		const QPoint seed = stack.pop();
		painter.drawPoint(seed);

		int x = seed.x();
		const int y = seed.y();

		for (; x <= x_max && !boundPixel(x, y); ++x)
			painter.drawPoint(x, y);
		const int x_right = x - 1;

		for (x = seed.x() - 1; x >= x_min && !boundPixel(x, y); --x)
			painter.drawPoint(x, y);
		const int x_left = x + 1;

		pushNewSeed(stack, y + 1, x_left, x_right);
		pushNewSeed(stack, y - 1, x_left, x_right);

		if (ui->delayCheckBox->isChecked()) {
			displayImage();
			delay(delay_value);
		}
		else
			image = pixmap.toImage();
	}

	displayImage();
}

bool MainWindow::boundPixel(int x, int y)
{
	return image.pixelColor(x, y) == defaultBoundColor;
}

bool MainWindow::cleanPixel(int x, int y)
{
	const QColor color = image.pixelColor(x, y);
	return color != defaultBoundColor && color != fillColor;
}

void MainWindow::pushNewSeed(QStack<QPoint> &stack, int y, int x_left, int x_right)
{
	if (y < y_min || y > y_max)
		return;

	for (int x = x_left; x <= x_right;) {
		bool flag = false;
		for (; cleanPixel(x, y); ++x)
			flag = true;
		if (flag)
			stack.push(QPoint(
				x == x_right && cleanPixel(x, y) ? x : x - 1,
				y
			));
		flag = false;
		for (; x < x_right && !cleanPixel(x, y); ++x)
			flag = true;
		if (!flag)
			++x;
	}
}

void MainWindow::on_clearPushButton_clicked()
{
	clearImage();
	closed = false;
	points.clear();
	edges.clear();
	start_point = 0;
	ui->tableWidget->clearContents();
	ui->tableWidget->model()->removeRows(0, ui->tableWidget->rowCount());
}

void MainWindow::on_setColorPushButton_clicked()
{
	fillColor = QColorDialog::getColor(fillColor, this, "Pick a color", QColorDialog::DontUseNativeDialog);
	if (fillColor == defaultBoundColor) {
		QMessageBox::critical(this, "Error", "Fill color can't be equal to edge color");
		fillColor = defaultFillColor;
	}
	colorLabel();
}

typename MainWindow::DrawType MainWindow::drawType()
{
	DrawType drawType = DrawType::none;
	switch (QApplication::keyboardModifiers()) {
	case Qt::ShiftModifier:
		drawType = DrawType::horizontal;
		break;
	case Qt::ControlModifier:
		drawType = DrawType::vertical;
		break;
	case Qt::ShiftModifier | Qt::ControlModifier:
		drawType = DrawType::diagonal;
		break;
	default:
		break;
	}

	return drawType;
}

void MainWindow::addPoint(const QPoint &point, DrawType drawType)
{
	const int n = points.size();
	points.push_back(point);
	ui->tableWidget->insertRow(n);

	if (n && !closed)
		switch (drawType) {
		case DrawType::horizontal:
			points[n].setY(points[n - 1].y());
			break;
		case DrawType::vertical:
			points[n].setX(points[n - 1].x());
			break;
		case DrawType::diagonal: {
			const int dx = points[n].x() - points[n - 1].x();
			const int dy = points[n].y() - points[n - 1].y();
			if (dx * dy >= 0) {
				const int d = (dx + dy) / 2;
				points[n].setX(points[n - 1].x() + d);
				points[n].setY(points[n - 1].y() + d);
			}
			else {
				const int d = (dx - dy) / 2;
				points[n].setX(points[n - 1].x() + d);
				points[n].setY(points[n - 1].y() - d);
			}
		}
		case DrawType::none:
		default:
			break;
		}

	ui->tableWidget->setItem(n, 0, new QTableWidgetItem(QString::number(points[n].x())));
	ui->tableWidget->setItem(n, 1, new QTableWidgetItem(QString::number(points[n].y())));

	if (closed) {
		start_point = n;
		closed = false;
	}
	else if (n)
		addEdge(QLine(points[closed ? 0 : n - 1], points[n]));
}

void MainWindow::addEdge(const QLine &edge)
{
	edges.push_back(edge);

	QPainter painter(&pixmap);
	painter.drawLine(edge);
	displayImage();
}

void MainWindow::delay(int times)
{
	for (int i = 0; i < times; i++) {
		repaint();
		resize(width(), height());
	}
}

void MainWindow::clearImage()
{
	pixmap.fill();
	displayImage();
}

void MainWindow::displayImage()
{
	ui->drawLabel->update();
	image = pixmap.toImage();
}

void MainWindow::colorLabel()
{
	QPalette palette = ui->colorLabel->palette();
	palette.setColor(ui->colorLabel->backgroundRole(), fillColor);
	ui->colorLabel->setAutoFillBackground(true);
	ui->colorLabel->setPalette(palette);
}
