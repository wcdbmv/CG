#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QGraphicsScene>

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
	void imageLabel();

	void dda(const QLine &line);
	void bresenhamFloat(const QLine &line);
	void bresenhamInteger(const QLine &line);
	void bresenhamAntialiased(const QLine &line);
	void defaultQt(const QLine &line);
	void wu(const QLine &line);

private slots:
	void on_fgPushButton_clicked();
	void on_setBGColorToFGPushButton_clicked();
	void on_drawLinePushButton_clicked();
	void on_drawSunPushButton_clicked();
	void on_clearAllPushButton_clicked();

private:
	Ui::MainWindow *ui;

	QColor fgColor;
	const QColor defaultBgColor = QColor(Qt::darkGray);
	const QColor defaultFgColor = QColor(Qt::red);

	QImage image;
	QGraphicsScene *scene;
};

#endif // MAINWINDOW_H
