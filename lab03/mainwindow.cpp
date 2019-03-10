#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QColorDialog>

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow),
	fgColor(Qt::black),
	bgColor(Qt::white)
{
	ui->setupUi(this);

	colorLabel(ui->fgLabel, fgColor);
	colorLabel(ui->bgLabel, bgColor);
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

void MainWindow::on_fgPushButton_clicked()
{
	QColor color = QColorDialog::getColor(fgColor, this, "Pick a FG color", QColorDialog::DontUseNativeDialog);
	colorLabel(ui->fgLabel, color);
}

void MainWindow::on_bgPushButton_clicked()
{
	QColor color = QColorDialog::getColor(bgColor, this, "Pick a BG color", QColorDialog::DontUseNativeDialog);
	colorLabel(ui->bgLabel, color);
}

void MainWindow::on_drawLinePushButton_clicked()
{
	//
}

void MainWindow::on_drawSunPushButton_clicked()
{
	//
}
