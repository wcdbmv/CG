#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QColorDialog>
#include <QElapsedTimer>
#include <cmath>

#include "line.h"
#include "dialog.h"

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

bool MainWindow::drawLine(const QLine &line, Canvas &canvas) {
	if (ui->ddaRadioButton->isChecked())
		return dda(line, canvas);
	else if (ui->bresenhamFloatRadioButton->isChecked())
		return bresenhamFloat(line, canvas);
	else if (ui->bresenhamIntegerRadioButton->isChecked())
		return bresenhamInteger(line, canvas);
	else if (ui->bresenhamAntialiasedRadioButton->isChecked())
		return bresenhamAntialiased(line, canvas);
	else if (ui->defaultQtRadioButton->isChecked())
		return defaultQt(line, canvas);
	else if (ui->wuRadioButton->isChecked())
		return wu(line, canvas);
	return true;
}

void MainWindow::drawPoint(const QPoint &point)
{
	QPixmap pixmap = QPixmap::fromImage(image);
	QPainter painter(&pixmap);
	painter.setPen(Qt::red);

	painter.drawEllipse(point, 3, 3);

	painter.end();
	image = pixmap.toImage();
}

void MainWindow::on_drawLinePushButton_clicked()
{
	const int x1 = 360 + ui->x1SpinBox->text().toInt();
	const int y1 = 360 - ui->y1SpinBox->text().toInt();
	const int x2 = 360 + ui->x2SpinBox->text().toInt();
	const int y2 = 360 - ui->y2SpinBox->text().toInt();
	const QLine line(x1, y1, x2, y2);

	Canvas canvas = { &image, &fgColor };

	QElapsedTimer timer;
	timer.start();

	drawLine(line, canvas);

	ui->statusBar->showMessage(QString::number(timer.nsecsElapsed() / 1000.0) + " μs");

	imageView();
}

static inline double toRadians(double x) { return x * M_PIl / 180; }

void MainWindow::on_drawSunPushButton_clicked()
{
	int length = ui->lengthSpinBox->text().toInt();
	int dangle = ui->angleSpinBox->text().toInt();
	Canvas canvas = { &image, &fgColor };
	for (int angle = 0; angle < 360; angle += dangle) {
		const int x2 = 360 + round(length * cos(toRadians(angle)));
		const int y2 = 360 - round(length * sin(toRadians(angle)));
		if (!drawLine(QLine(360, 360, x2, y2), canvas))
			drawPoint(QPoint(x2, y2));
	}
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

void MainWindow::on_statisticsPushButton_clicked()
{
	const int N = 1000;
	const int M = 100;
	double ns[6][M];

	QImage image(721, 721, QImage::Format_ARGB32);
	image.fill(defaultBgColor);
	Canvas canvas = { &image, &fgColor };

	bool (*f[5])(const QLine &, Canvas &) = {
		dda,
		bresenhamFloat,
		bresenhamInteger,
		bresenhamAntialiased,
		wu
	};

	for (int i = 0; i != 5; ++i) {
		for (int j = 1; j != M + 1; ++j) {
			QElapsedTimer timer;
			timer.start();

			for (int k = 0; k != N; ++k)
				f[i](QLine(360, 360, 360 + j, 360 - j), canvas);

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
					defaultQtCore(QLine(360, 360, 360 + j, 360 - j), painter);

				ns[5][j - 1] = static_cast<double>(timer.nsecsElapsed()) / N;
			}
		}
		painter.end();
	}

	QVector<QVector<double>> qns(6);
	for (int i = 0; i != 6; ++i)
		std::copy(ns[i], ns[i] + M, std::back_inserter(qns[i]));

	Dialog dialog(qns, M);
	dialog.setModal(true);
	dialog.exec();
}
