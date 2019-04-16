#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QGraphicsScene>

#include "canvas.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

private:
	void colorLabel(QLabel *label, QColor &color);
	void imageView();

	void drawCircle(const QPoint &center, int radius, Canvas &canvas);
	void drawEllipse(const QPoint &center, int a, int b, Canvas &canvas);

private slots:
	void on_setFgColorPushButton_clicked();
	void on_setBgColorToFgPushButton_clicked();
	void on_drawCirclePushButton_clicked();
	void on_drawEllipsePushButton_clicked();
	void on_drawCirclesPushButton_clicked();
	void on_drawEllipsesPushButton_clicked();
	void on_clearAllPushButton_clicked();
	void on_setDefaultFgColor_clicked();

private:
	Ui::MainWindow *ui;

	const QColor defaultBgColor = QColor(Qt::white);
	const QColor defaultFgColor = QColor(Qt::black);
	QColor fgColor;

	QGraphicsScene *scene;
	QImage image;
};

#endif // MAINWINDOW_H
