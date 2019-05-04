#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QColorDialog>
#include <QMessageBox>
#include <QPainter>
#include <algorithm>

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow),
	closed(false),
	n_edges(0)
{
	ui->setupUi(this);

	pixmap = QPixmap(ui->drawLabel->width(), ui->drawLabel->height());
	ui->drawLabel->setPixmapPointer(pixmap);

	clearImage();
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
	const int x = event->x() - ui->drawLabel->x();
	const int y = event->y() - ui->drawLabel->y();
	if (x < 0 || y < 0 || x >= pixmap.width() || y >= pixmap.height())
		return;

	DrawType drawType = DrawType::none;
	switch (QApplication::keyboardModifiers()) {
	case Qt::ShiftModifier:
		drawType = DrawType::horizontal;
		break;
	case Qt::ControlModifier:
		drawType = DrawType::vertical;
		break;
	case Qt::ShiftModifier | Qt::ControlModifier:
		drawType = DrawType::diagonal;
		break;
	default:
		break;
	}

	addPoint(QPoint(x, y), drawType);
}

void MainWindow::on_addPointPushButton_clicked()
{
	const int x = ui->xSpinBox->text().toInt();
	const int y = ui->ySpinBox->text().toInt();
	addPoint(QPoint(x, y), DrawType::none);
}

void MainWindow::on_closePushButton_clicked()
{
	if (closed) {
		QMessageBox::critical(this, "Error", "Already closed");
		return;
	}

	if (n_edges < 2) {
		QMessageBox::critical(this, "Error", "Need at least 2 edges");
		return;
	}

	addEdge(QLine(points[points.size() - 1], points[0]));
	closed = true;
	n_edges = 0;
}

constexpr int sgn(int val) {
	if (val > 0)
		return 1;
	if (val < 0)
		return -1;
	return 0;
}

void handleVertices(QVector<QPoint> &intersections, const QVector<QLine> &edges)
{
	for (int i = 0; i < edges.size(); ++i) {
		if (edges[i].p1().y() != edges[i].p2().y()) {
			intersections.push_back(edges[i].p2());
			int j = (i + 1) % edges.size(), skip = 0;
			for (; edges[j].p1().y() == edges[j].p2().y(); j = (j + 1) % edges.size(), ++skip)
				;
			if (sgn(edges[i].p1().y() - edges[i].p2().y())
			 == sgn(edges[j].p2().y() - edges[j].p1().y()))
				intersections.push_back(edges[i].p2());
			i += skip;
		}
	}
}

void sortY(QVector<QPoint> &intersections)
{
	std::sort(
		intersections.begin(), intersections.end(),
		[](const QPoint &p1, const QPoint &p2) {
			return p1.y() > p2.y();
	});
}

void sortX(QVector<QPoint> &intersections, QVector<int> &indices)
{
	indices.reserve(intersections.front().y() - intersections.back().y() + 1);
	for (int i = 0; i < intersections.size();) {
		indices.push_back(i);
		int j = i;
		while (++j < intersections.size() && intersections[j].y() == intersections[i].y())
			;
		std::sort(
			intersections.begin() + i, intersections.begin() + j,
			[](const QPoint &p1, const QPoint &p2) {
				return p1.x() < p2.x();
		});
		i = j;
	}
	indices.push_back(intersections.size());
}

void MainWindow::on_fillPushButton_clicked()
{
	if (!closed) {
		QMessageBox::critical(this, "Error", "Figure is not closed");
		return;
	}

	handleVertices(intersections, edges);
	displayImage();

	sortY(intersections);

	QVector<int> indices;
	sortX(intersections, indices);

	QPainter painter(&pixmap);
	for (int k = 0; k < indices.size() - 1; ++k) {
		const int ix_begin = indices[k];
		const int ix_end = indices[k + 1];
		for (int i = ix_begin; i < ix_end - 1; i += 2)
			painter.drawLine(intersections[i], intersections[i + 1]);
		if (ui->delayCheckBox->isChecked()) {
			displayImage();
			delay(ui->delaySpinBox->value());
		}
	}

	displayImage();
}

void MainWindow::on_clearPushButton_clicked()
{
	clearImage();
	closed = false;
	points.clear();
	edges.clear();
	intersections.clear();
	n_edges = 0;
	ui->tableWidget->clearContents();
	ui->tableWidget->model()->removeRows(0, ui->tableWidget->rowCount());
}

void MainWindow::addPoint(const QPoint &point, DrawType drawType)
{
	const int n = points.size();
	points.push_back(point);
	ui->tableWidget->insertRow(n);

	if (n)
		switch (drawType) {
		case DrawType::horizontal:
			points[n].setY(points[n - 1].y());
			break;
		case DrawType::vertical:
			points[n].setX(points[n - 1].x());
			break;
		case DrawType::diagonal: {
			const int dx = points[n].x() - points[n - 1].x();
			const int dy = points[n].y() - points[n - 1].y();
			if (dx * dy >= 0) {
				const int d = (dx + dy) / 2;
				points[n].setX(points[n - 1].x() + d);
				points[n].setY(points[n - 1].y() + d);
			}
			else {
				const int d = (dx - dy) / 2;
				points[n].setX(points[n - 1].x() + d);
				points[n].setY(points[n - 1].y() - d);
			}
		}
		case DrawType::none:
		default:
			break;
		}

	ui->tableWidget->setItem(n, 0, new QTableWidgetItem(QString::number(points[n].x())));
	ui->tableWidget->setItem(n, 1, new QTableWidgetItem(QString::number(points[n].y())));

	if (n) {
		addEdge(QLine(points[closed ? 0 : n - 1], points[n]));
		closed = false;
	}
}

void MainWindow::addEdge(const QLine &edge)
{
	edges.push_back(edge);
	++n_edges;

	dda(edge);

	displayImage();
}

void MainWindow::dda(const QLine &edge)
{
	const int deltaX = edge.p2().x() - edge.p1().x();
	const int deltaY = edge.p2().y() - edge.p1().y();

	int length = qMax(qAbs(deltaX), qAbs(deltaY));

	QPainter painter(&pixmap);

	const bool horizontal = edge.p1().y() == edge.p2().y();
	if (horizontal && edge.p1().x() == edge.p1().y()) {
		painter.drawPoint(edge.p1());
		return;
	}

	// Полагаем большее из приращений dx или dy равным единице растра
	const float dx = static_cast<float>(deltaX) / length;
	const float dy = static_cast<float>(deltaY) / length;

	int xl = -1;
	int xr = -1;
	int yp = -1;
	const int xr_dir = sgn(deltaX);

	float xf = edge.p1().x();
	float yf = edge.p1().y();

	for (int i = 0; i <= length; ++i) {
		const int x = qRound(xf);
		const int y = qRound(yf);
		painter.drawPoint(x, y);
		if (!horizontal) {
			if (y != yp) {
				if (yp != edge.p1().y() && i)
					intersections.push_back(QPoint((xl + xr) / 2, yp));
				xl = xr = x;
				yp = y;
			}
			else
				xr += xr_dir;
		}
		xf += dx;
		yf += dy;
	}
}

void MainWindow::delay(int times)
{
	for (int i = 0; i < times; ++i) {
		repaint();
		resize(width(), height());
	}
}

void MainWindow::clearImage()
{
	pixmap.fill();
	displayImage();
}

void MainWindow::displayImage()
{
	ui->drawLabel->update();
}
