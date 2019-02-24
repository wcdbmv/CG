#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , j_max(0)
{
	ui->setupUi(this);

	connect(ui->tableWidget, SIGNAL(itemChanged(QTableWidgetItem*)),
	        this, SLOT(tableWidgetItem_changed(QTableWidgetItem*)));
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::on_addPointPushButton_clicked()
{
	bool ok;
	double x = ui->xLineEdit->text().toDouble(&ok);
	if (!ok) {
		ui->statusBar->showMessage("Invalid x", STATUS_BAR_TIMEOUT);
		return;
	}
	double y = ui->yLineEdit->text().toDouble(&ok);
	if (!ok) {
		ui->statusBar->showMessage("Invalid y", STATUS_BAR_TIMEOUT);
		return;
	}
	Point point(x, y);
	if (points.contains(point)) {
		ui->statusBar->showMessage("Point already exists", STATUS_BAR_TIMEOUT);
		return;
	}

	j_max = 0;
	points.push_back(point);

	int i = ui->tableWidget->rowCount();
	ui->tableWidget->insertRow(i);
	ui->tableWidget->setItem(i, 0, new QTableWidgetItem(QString::number(x)));
	ui->tableWidget->setItem(i, 1, new QTableWidgetItem(QString::number(y)));

	ui->statusBar->showMessage("Point added successful", STATUS_BAR_TIMEOUT);
}

void MainWindow::on_deletePointPushButton_clicked()
{
	bool ok;
	int i = ui->iLineEdit->text().toInt(&ok);
	if (!ok) {
		ui->statusBar->showMessage("Invalid index", STATUS_BAR_TIMEOUT);
		return;
	}
	if (i <= 0 || i > points.size()) {
		ui->statusBar->showMessage("Index out of range", STATUS_BAR_TIMEOUT);
		return;
	}

	j_max = 0;
	points.remove(i - 1);

	ui->tableWidget->removeRow(i - 1);

	ui->statusBar->showMessage("Point deleted successful", STATUS_BAR_TIMEOUT);
}

void MainWindow::tableWidgetItem_changed(QTableWidgetItem *item)
{
	bool ok;
	double coord = item->text().toDouble(&ok);
	if (!ok) {
		if (item->column())
			ui->statusBar->showMessage("Invalid y", STATUS_BAR_TIMEOUT);
		else
			ui->statusBar->showMessage("Invalid x", STATUS_BAR_TIMEOUT);
		item->setText(QString::number(points[item->row()][item->column()]));
		return;
	}

	Point point(coord, coord);
	point[!item->column()] = points[item->row()][!item->column()];
	if (points.contains(point)) {
		ui->statusBar->showMessage("Point already exists", STATUS_BAR_TIMEOUT);
		item->setText(QString::number(points[item->row()][item->column()]));
		return;
	}

	j_max = 0;
	points[item->row()][item->column()] = coord;

	ui->statusBar->showMessage("Point edited successful", STATUS_BAR_TIMEOUT);
}

void MainWindow::on_calculatePushButton_clicked()
{
	if (j_max) {
		ui->statusBar->showMessage("Already calculated", STATUS_BAR_TIMEOUT);
		return;
	}
	if (points.size() < 3) {
		ui->statusBar->showMessage("Too few points", STATUS_BAR_TIMEOUT);
		return;
	}

	int i, j, k;
	angle_max = -1;
	Line y_axis(1, 0, 0);
	for (i = 0; i < points.size() - 2; ++i)
		for (j = i + 1; j < points.size() - 1; ++j)
			for (k = j + 1; k < points.size(); ++k)
				if (!on_one_line(points[i], points[j], points[k])) {
					Point orthocenter = Triangle(points[i], points[j], points[k]).orthocenter();
					double angle_cur = orthocenter == Point() ? 0.0 : angle(Line(orthocenter, Point()), y_axis);
					if (angle_cur > angle_max) {
						i_max = i, j_max = j, k_max = k;
						orthocenter_max = orthocenter;
						angle_max = angle_cur;
					}
				}

	if (!j_max) {
		ui->statusBar->showMessage("No one triangle can be drawn", STATUS_BAR_TIMEOUT);
	}

	ui->statusBar->showMessage("Triangle found successful", STATUS_BAR_TIMEOUT);
	update();
}

void MainWindow::on_clearPushButton_clicked()
{
	j_max = 0;
	points.clear();

	while (ui->tableWidget->rowCount())
		ui->tableWidget->removeRow(ui->tableWidget->rowCount() - 1);

	update();
}

QPointF MainWindow::coord(const Point &point) const
{
	return QPointF(PAINT_WIDTH / 2 + point.x * scale_factor, PAINT_HEIGHT / 2 - point.y * scale_factor);
}

void MainWindow::paintEvent(QPaintEvent */*event*/)
{
	QPainter painter(this);
	painter.translate(230, 0);

	// draw axes
	painter.fillRect(0, 0, PAINT_WIDTH, PAINT_HEIGHT, QBrush(Qt::white));
	painter.drawLine(0, PAINT_HEIGHT / 2, PAINT_WIDTH, PAINT_HEIGHT / 2);
	painter.drawLine(PAINT_WIDTH / 2, 0, PAINT_WIDTH / 2, PAINT_HEIGHT);
	painter.drawPolyline(QPolygon() << QPoint(PAINT_WIDTH / 2 - 2, 10) << QPoint(PAINT_WIDTH / 2, 0) << QPoint(PAINT_WIDTH / 2 + 2, 10));
	painter.drawPolyline(QPolygon() << QPoint(PAINT_WIDTH - 10, PAINT_HEIGHT / 2 - 2) << QPoint(PAINT_WIDTH, PAINT_HEIGHT / 2) << QPoint(PAINT_WIDTH - 10, PAINT_HEIGHT / 2 + 2));

	if (!j_max)
		return;

	// find scale factor
	double x_max = qMax(qMax(qAbs(points[i_max].x), qAbs(points[j_max].x)), qMax(qAbs(points[k_max].x), qAbs(orthocenter_max.x)));
	double y_max = qMax(qMax(qAbs(points[i_max].y), qAbs(points[j_max].y)), qMax(qAbs(points[k_max].y), qAbs(orthocenter_max.y)));
	double scale_x = 9.0 * PAINT_WIDTH / (20.0 * x_max);
	double scale_y = 9.0 * PAINT_HEIGHT / (20.0 * y_max);
	scale_factor = qMin(scale_x, scale_y);

	// draw points
	painter.setPen(Qt::red);
	painter.setBrush(Qt::red);
	foreach (const Point &point, points) {
		painter.drawEllipse(coord(point), 4, 4);
	}

	// draw triangle
	painter.setPen(QPen(Qt::green, 3));
	painter.setBrush(QBrush(Qt::Dense4Pattern));
	painter.drawPolygon(QPolygonF() << coord(points[i_max]) << coord(points[j_max]) << coord(points[k_max]));

	Triangle triangle(points[i_max], points[j_max], points[k_max]);

	const QVector<int> indices = {
	    i_max,
	    j_max,
	    k_max
	};

	const QVector<Line> sides = {
	    Line(points[j_max], points[k_max]),
	    Line(points[k_max], points[i_max]),
	    Line(points[i_max], points[j_max])
	};

	const QVector<Line> altitudes = {
	    sides[0].perpendicular(points[i_max]),
	    sides[1].perpendicular(points[j_max]),
	    sides[2].perpendicular(points[k_max])
	};

	const QVector<Point> Hs = {
	    intersection(altitudes[0], sides[0]),
	    intersection(altitudes[1], sides[1]),
	    intersection(altitudes[2], sides[2])
	};

	painter.setPen(QPen(Qt::blue, 2));
	for (int i = 0; i != 3; ++i)
		painter.drawLine(coord(points[indices[i]]), coord(Hs[i]));

	if (!triangle.includes(orthocenter_max)) {
		for (int i = 0; i != 3; ++i)
			painter.drawLine(coord(Hs[i]), coord(orthocenter_max));

		painter.setPen(QPen(Qt::lightGray, 1));
		for (int i = 0; i != 3; ++i) {
			const Point &A = points[indices[(i + 1) % 3]];
			const Point &B = points[indices[(i + 2) % 3]];
			// tired
			if (Point(A, Hs[i]).norm() < Point(B, Hs[i]).norm())
				painter.drawLine(coord(A), coord(Hs[i]));
			else
				painter.drawLine(coord(B), coord(Hs[i]));
		}
	}

	painter.setPen(Qt::blue);
	painter.setBrush(Qt::blue);
	painter.drawEllipse(coord(orthocenter_max), 5, 5);

	painter.setPen(QPen(Qt::gray, 1));
	painter.setBrush(Qt::gray);
	if (orthocenter_max != Point())
		drawLine(Line(Point(), orthocenter_max), painter);
	// painter.drawLine(coord(Point()), coord(orthocenter_max));
	double startAngle, spanAngle;
	if (orthocenter_max.x >= 0 && orthocenter_max.y >= 0
	 || orthocenter_max.x <= 0 && orthocenter_max.y <= 0) {
		spanAngle = degrees(angle_max) * 16;
		startAngle = 90 * 16 - spanAngle;
	}
	else {
		startAngle = 90 * 16;
		spanAngle = degrees(angle_max) * 16;
	}
	painter.drawPie(PAINT_WIDTH / 2 - 25, PAINT_HEIGHT / 2 - 25, 50, 50, startAngle, spanAngle);
}

void MainWindow::drawLine(const Line &line, QPainter &painter)
{
	const QVector<Line> borders = {
	    Line(0, 1, - PAINT_HEIGHT / (2 * scale_factor)),
	    Line(1, 0, - PAINT_WIDTH / (2 * scale_factor)),
	    Line(0, 1, PAINT_HEIGHT / (2 * scale_factor)),
	    Line(1, 0, PAINT_WIDTH / (2 * scale_factor))
	};
	QVector<Point> borders_borders;
	foreach (const Line &border, borders) {
		if (parallel(line, border))
			continue;

		Point point = intersection(line, border);
		if ((- PAINT_HEIGHT / (2 * scale_factor) - EPS <= point.y && point.y <= PAINT_HEIGHT / (2 * scale_factor) + EPS)
		 && (- PAINT_WIDTH / (2 * scale_factor) - EPS <= point.x && point.x <= PAINT_WIDTH / (2 * scale_factor) + EPS)
		 && !borders_borders.contains(point)) {
			borders_borders.push_back(point);
		}
	}
	Q_ASSERT(borders_borders.size() == 2);
	painter.drawLine(coord(borders_borders[0]), coord(borders_borders[1]));
	borders_borders.clear();
}
