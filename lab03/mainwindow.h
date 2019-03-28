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
	void clearImage();
	void imageView();

	bool drawLine(const QLine &line, Canvas &canvas);
	void drawPoint(const QPoint &point);

private slots:
	void on_fgPushButton_clicked();
	void on_setBGColorToFGPushButton_clicked();
	void on_drawLinePushButton_clicked();
	void on_drawSunPushButton_clicked();
	void on_clearAllPushButton_clicked();
	void on_setDefaultFGColorPushButton_clicked();
	void on_statisticsPushButton_clicked();

private:
	Ui::MainWindow *ui;

	QColor fgColor;
	const QColor defaultBgColor = QColor(11, 11, 11);
	const QColor defaultFgColor = QColor(10, 200, 10);

	QImage image;
	QGraphicsScene *scene;
};

#endif // MAINWINDOW_H
