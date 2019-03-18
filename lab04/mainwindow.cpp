#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QColorDialog>
#include <QElapsedTimer>
#include <cmath>

#include "circle.h"
#include "ellipse.h"
#include "dialog.h"

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow),
	fgColor(defaultFgColor),
	scene(new QGraphicsScene(0, 0, 721, 721)),
	image(QImage(721, 721, QImage::Format_ARGB32))
{
	ui->setupUi(this);

	ui->graphicsView->setScene(scene);
	imageView();

	on_clearAllPushButton_clicked();
	on_setDefaultFgColor_clicked();
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::colorLabel(QLabel *label, QColor &color) {
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

void MainWindow::on_setFgColorPushButton_clicked()
{
	fgColor = QColorDialog::getColor(fgColor, this, "Pick a FG color", QColorDialog::DontUseNativeDialog);
	colorLabel(ui->fgLabel, fgColor);
}

void MainWindow::on_setBgColorToFgPushButton_clicked()
{
	fgColor = defaultBgColor;
	colorLabel(ui->fgLabel, fgColor);
}

void MainWindow::on_setDefaultFgColor_clicked()
{
	fgColor = defaultFgColor;
	colorLabel(ui->fgLabel, fgColor);
}

void MainWindow::drawCircle(const QPoint &center, int radius, Canvas &canvas)
{
	if (ui->canonicalRadioButton->isChecked())
		canonical(center, radius, canvas);
	else if (ui->parametricRadioButton->isChecked())
		parametric(center, radius, canvas);
	else if (ui->bresenhamRadioButton->isChecked())
		bresenham(center, radius, canvas);
	else if (ui->midPointRadioButton->isChecked())
		midPoint(center, radius, canvas);
	else if (ui->defaultQtRadioButton->isChecked())
		defaultQt(center, radius, canvas);
}

void MainWindow::drawEllipse(const QPoint &center, int a, int b, Canvas &canvas)
{
	if (ui->canonicalRadioButton->isChecked())
		canonical(center, a, b, canvas);
	else if (ui->parametricRadioButton->isChecked())
		parametric(center, a, b, canvas);
	else if (ui->bresenhamRadioButton->isChecked())
		bresenham(center, a, b, canvas);
	else if (ui->midPointRadioButton->isChecked())
		midPoint(center, a, b, canvas);
	else if (ui->defaultQtRadioButton->isChecked())
		defaultQt(center, a, b, canvas);
}

void MainWindow::on_drawCirclePushButton_clicked()
{
	const int x = 360 + ui->xSpinBox->text().toInt();
	const int y = 360 - ui->ySpinBox->text().toInt();
	const int r = ui->rSpinBox->text().toInt();
	const QPoint center(x, y);
	Canvas canvas = { &image, &fgColor };

	QElapsedTimer timer;
	timer.start();

	drawCircle(center, r, canvas);

	ui->statusBar->showMessage(QString::number(timer.nsecsElapsed()));

	imageView();
}

void MainWindow::on_drawEllipsePushButton_clicked()
{
	const int x = 360 + ui->xSpinBox->text().toInt();
	const int y = 360 - ui->ySpinBox->text().toInt();
	const int a = ui->aSpinBox->text().toInt();
	const int b = ui->bSpinBox->text().toInt();
	const QPoint center(x, y);
	Canvas canvas = { &image, &fgColor };

	QElapsedTimer timer;
	timer.start();

	drawEllipse(center, b, a, canvas);

	ui->statusBar->showMessage(QString::number(timer.nsecsElapsed()));

	imageView();
}

void MainWindow::on_drawCirclesPushButton_clicked()
{
	int r0 = ui->r0SpinBox->text().toInt();
	const int dr = ui->drSpinBox->text().toInt();
	const int n = ui->nSpinBox->text().toInt();
	const QPoint center(360, 360);
	Canvas canvas = { &image, &fgColor };

	for (int i = 0; i != n; ++i) {
		drawCircle(center, r0, canvas);
		r0 += dr;
	}

	imageView();
}

void MainWindow::on_drawEllipsesPushButton_clicked()
{
	int a = ui->a0SpinBox->text().toInt();
	int b = ui->b0SpinBox->text().toInt();
	const int dr = ui->drSpinBox->text().toInt();
	const int n = ui->nSpinBox->text().toInt();
	const QPoint center(360, 360);
	Canvas canvas = { &image, &fgColor };

	for (int i = 0; i != n; ++i) {
		drawEllipse(center, b, a, canvas);
		a += dr;
		b += dr;
	}

	imageView();
}

void MainWindow::on_clearAllPushButton_clicked()
{
	ui->statusBar->showMessage("");
	image.fill(defaultBgColor);
	imageView();
}

/* this is so ugly i'm so tired don't look */
void MainWindow::on_statisticsPushButton_clicked()
{
	const int N = 50;
	const int M = 100;
	double ns[10][M];

	QImage image(721, 721, QImage::Format_ARGB32);
	image.fill(defaultBgColor);
	Canvas canvas = { &image, &fgColor };
	QPoint center(360, 360);

	void (*f[4])(const QPoint &, int, Canvas &) = {
		canonical,
		parametric,
		bresenham,
		midPoint
	};

	for (int i = 0; i != 4; ++i) {
		for (int j = 1; j != M + 1; ++j) {
			QElapsedTimer timer;
			timer.start();

			for (int k = 0; k != N; ++k)
				f[i](center, j, canvas);

			ns[i][j - 1] = static_cast<double>(timer.nsecsElapsed()) / N;
		}
	}

	{
		QPixmap pixmap = QPixmap::fromImage(image);
		QPainter painter(&pixmap);
		painter.setPen(fgColor);
		{
			for (int j = 1; j != M + 1; ++j) {
				QElapsedTimer timer;
				timer.start();

				for (int k = 0; k != N; ++k)
					defaultQtCore(center, j, painter);

				ns[4][j - 1] = static_cast<double>(timer.nsecsElapsed()) / N;
			}
		}
		painter.end();
	}

	image.fill(defaultBgColor);

	void (*g[4])(const QPoint &, int, int, Canvas &) = {
		canonical,
		parametric,
		bresenham,
		midPoint
	};

	for (int i = 0; i != 4; ++i) {
		for (int j = 2; j != M + 2; ++j) {
			QElapsedTimer timer;
			timer.start();

			for (int k = 0; k != N; ++k)
				g[i](center, j >> 1, j << 1, canvas);

			ns[i + 5][j - 2] = static_cast<double>(timer.nsecsElapsed()) / N;
		}
	}

	{
		QPixmap pixmap = QPixmap::fromImage(image);
		QPainter painter(&pixmap);
		painter.setPen(fgColor);
		{
			for (int j = 1; j != M + 1; ++j) {
				QElapsedTimer timer;
				timer.start();

				for (int k = 0; k != N; ++k)
					defaultQtCore(center, j >> 1, j << 1, painter);

				ns[9][j - 1] = static_cast<double>(timer.nsecsElapsed()) / N;
			}
		}
		painter.end();
	}

	QVector<QVector<double>> qns(10);
	for (int i = 0; i != 10; ++i)
		std::copy(ns[i], ns[i] + M, std::back_inserter(qns[i]));

	Dialog dialog(qns, M);
	dialog.setModal(true);
	dialog.exec();
}
