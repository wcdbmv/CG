#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include <QPainter>
#include "transform.h"

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
	void on_translatePushButton_clicked();
	void on_scalePushButton_clicked();
	void on_rotatePushButton_clicked();
	void on_undoPushButton_clicked();
	void on_clearPushButton_clicked();

	bool get_var(double &var, const QLineEdit *lineEdit, const QString &err_msg);

private:
	void initPoints();
	double x_coord(double x) const;
	double y_coord(double y) const;
	QPointF coord(const Point &point) const;
	QPen choosePen(int i) const;

private:
	Ui::MainWindow *ui;

	static const int STATUS_BAR_TIMEOUT = 5000;
	static const int PAINT_HEIGHT = 991;
	static const int PAINT_WIDTH = 1690;
	static const int NORMAL_SCALE_FACTOR = 35;
	double compress = 1;

	double a = 2.5;
	double b = 1;

	double x, y;
	double kx, ky;
	double alpha;

	int n = 1000;
	QVector<Point> points;
	QVector<Transform> transforms;
};

#endif // MAINWINDOW_H
