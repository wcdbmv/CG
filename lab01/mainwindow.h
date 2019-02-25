#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include <QTableWidgetItem>
#include <QLineEdit>
#include <QPainter>
#include "geometry.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

	virtual void paintEvent(QPaintEvent *event);

private slots:
	void on_addPointPushButton_clicked();
	void on_deletePointPushButton_clicked();
	void on_calculatePushButton_clicked();
	void on_clearPushButton_clicked();

	void tableWidgetItem_changed(QTableWidgetItem *item);

private:
	bool get_var(double &var, const QLineEdit *lineEdit, const QString &err_msg);

	double x_coord(double x) const;
	double y_coord(double y) const;
	QPointF coord(const Point &point) const;
	void drawLine(const Line &line, QPainter &painter);
	QPen choosePen(int i) const;

private:
	Ui::MainWindow *ui;

	QVector<Point> points;
	int i_max, j_max, k_max;
	double angle_max;
	Point orthocenter_max;
	double scale_factor;

	static const int STATUS_BAR_TIMEOUT = 5000;
	static const int PAINT_WIDTH = 1690;
	static const int PAINT_HEIGHT = 991;
};

#endif // MAINWINDOW_H
