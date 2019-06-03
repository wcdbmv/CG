#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMouseEvent>
#include <QPixmap>
#include <QLabel>

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
	void on_addPolygonVertexPushButton_clicked();
	void on_addClipperVertexPushButton_clicked();
	void on_setPolygonColorPushButton_clicked();
	void on_setClipperColorPushButton_clicked();
	void on_setClippedLineColorPushButton_clicked();
	void on_closePolygonPushButton_clicked();
	void on_closeClipperPushButton_clicked();
	void on_clipPushButton_clicked();
	void on_deletePolygonPushButton_clicked();
	void on_deleteClipperPushButton_clicked();
	void on_clearAllPushButton_clicked();

protected:
	void mousePressEvent(QMouseEvent *event);
	void receiveKeyboardModifiers(int &x, int &y, int x1, int y1);

private:
	Ui::MainWindow *ui;

	QPixmap pixmap;
	void displayImage();

	QColor polygonColor;
	QColor clipperColor;
	QColor clippedLineColor;
	void setColor(QColor &color, const QString &msg, QLabel *label);
	void setPolygonColor(QColor color);
	void setClipperColor(QColor color);
	void setClippedLineColor(QColor color);
	void colorLabel(QColor color, QLabel *label);

	QVector<QPoint> polygonVertices;
	QVector<QPoint> clipperVertices;
	bool polygonClosed;
	bool clipperClosed;
	void addFigureVertex(QVector<QPoint> &figure_vertices, bool &figure_closed, const QPoint& vertex);
	void closeFigure(int figure_vertices_size, bool &figure_closed);
	void deleteFigure(QVector<QPoint> &figure_vertices, bool &figure_closed);

	int checkClipper();
	QVector<QPoint> clipPolygon(int direction);
	bool checkIntersection(const QPoint &sp, const QPoint &ep, const QPoint &p0, const QPoint &p1);
	int isVisible(const QPoint &p, const QPoint &p1, const QPoint &p2);
	QPoint intersection(QPoint &p1, QPoint &p2, QPoint &cp1, QPoint &cp2);
};

#endif // MAINWINDOW_H
