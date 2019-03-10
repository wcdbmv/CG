#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
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

private:
	void colorLabel(QLabel *label, QColor &color);

private slots:
	void on_fgPushButton_clicked();
	void on_bgPushButton_clicked();
	void on_drawLinePushButton_clicked();
	void on_drawSunPushButton_clicked();

private:
	Ui::MainWindow *ui;

	QColor fgColor, bgColor;
};

#endif // MAINWINDOW_H
