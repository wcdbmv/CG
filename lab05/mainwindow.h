#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QMainWindow>
#include <QMouseEvent>
#include <QVector>
#include <QPixmap>
#include <QImage>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

private slots:
	void mousePressEvent(QMouseEvent *event);
	void on_addPointPushButton_clicked();
	void on_closePushButton_clicked();
	void on_fillPushButton_clicked();
	void on_clearPushButton_clicked();

private:
	Ui::MainWindow *ui;

	bool closed;
	QVector<QPoint> points;
	QVector<QLine> edges;

	QPixmap bitmap;
	QImage image;
	QGraphicsScene *scene;
	QGraphicsPixmapItem *bitmap_item;

	enum DrawType {	none, horizontal, vertical, diagonal };
	void addPoint(const QPoint &point, DrawType drawType);
	void addEdge(const QLine &edge);

	void clearImage();
	void displayImage();
};

#endif // MAINWINDOW_H
