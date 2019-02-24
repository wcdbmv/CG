#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QVector>
#include <cmath>

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	initPoints();
}

MainWindow::~MainWindow()
{
	delete ui;
}

bool MainWindow::get_var(double &var, const QLineEdit *lineEdit, const QString &err_msg)
{
	bool ok;
	var = lineEdit->text().toDouble(&ok);
	if (!ok)
		ui->statusBar->showMessage(err_msg, STATUS_BAR_TIMEOUT);
	return ok;
}

void MainWindow::on_translatePushButton_clicked()
{
	if (!(get_var(x, ui->xLineEdit, "Invalid x")
	   && get_var(y, ui->yLineEdit, "Invalid y")))
		return;

	Transform translation;
	translation.translate(Point(x, y));
	transforms.push_back(translation);

	ui->statusBar->showMessage("Translated successful", STATUS_BAR_TIMEOUT);
	update();
}

void MainWindow::on_scalePushButton_clicked()
{
	double x, y, kx, ky;
	if (!(get_var(x, ui->xLineEdit, "Invalid x")
	   && get_var(y, ui->yLineEdit, "Invalid y")
	   && get_var(kx, ui->kxLineEdit, "Invalid kx")
	   && get_var(ky, ui->kyLineEdit, "Invalid ky")))
		return;

	Transform scaling;
	scaling.scale(Point(kx, ky), Point(x, y));
	transforms.push_back(scaling);

	ui->statusBar->showMessage("Scaled successful", STATUS_BAR_TIMEOUT);
	update();
}

void MainWindow::on_rotatePushButton_clicked()
{
	double x, y, alpha;
	if (!(get_var(x, ui->xLineEdit, "Invalid x")
	   && get_var(y, ui->yLineEdit, "Invalid y")
	   && get_var(alpha, ui->alphaLineEdit, "Invalid alpha")))
		return;

	Transform rotation;
	rotation.rotate(alpha * M_PIl / 180, Point(x, y));
	transforms.push_back(rotation);

	ui->statusBar->showMessage("Rotated successful", STATUS_BAR_TIMEOUT);
	update();
}

void MainWindow::on_undoPushButton_clicked()
{
	if (transforms.empty()) {
		ui->statusBar->showMessage("There are no transforms");
		return;
	}

	transforms.pop_back();

	ui->statusBar->showMessage("Undo successful", STATUS_BAR_TIMEOUT);
	update();
}

void MainWindow::on_clearPushButton_clicked()
{
	transforms.clear();
	update();
}

void MainWindow::initPoints()
{
	points = QVector<Point>(n + 4);

	points[0] = Point(8, 0);
	points[1] = Point(0, 3);
	points[2] = Point(-8, 0);
	points[3] = Point(0, -3);

	for (int i = 0; i < n; ++i) {
		double t = 2 * M_PIl * i / n;
		double cost = std::cos(t);
		double sint = std::sin(t);
		points[i + 4] = Point(
			a * cost * cost + b * cost - a + b,
			a * cost * sint + b * sint
		);
	}
}

double MainWindow::x_coord(double x) const
{
	return PAINT_WIDTH / 2 + x * NORMAL_SCALE_FACTOR / compress;
}

double MainWindow::y_coord(double y) const
{
	return PAINT_HEIGHT / 2 - y * NORMAL_SCALE_FACTOR / compress;
}

QPointF MainWindow::coord(const Point &point) const
{
	return QPointF(x_coord(point.x), y_coord(point.y));
}

QPen MainWindow::choosePen(int i) const
{
	if (i % 50 == 0)
		return QPen(Qt::black);
	if (i % 10 == 0)
		return QPen(Qt::darkGray);
	if (i % 5 == 0)
		return QPen(Qt::gray);
	return QPen(Qt::lightGray);
}

void MainWindow::paintEvent(QPaintEvent *)
{
	QPainter painter(this);
	painter.translate(230, 0);

	painter.fillRect(0, 0, PAINT_WIDTH, PAINT_HEIGHT, QBrush(Qt::white));

	Transform transform;
	foreach (const Transform &cur, transforms) {
		transform.combine(cur);
	}

	compress = 1;

	const Point paint_size(
		PAINT_WIDTH / (2.0 * NORMAL_SCALE_FACTOR),
		PAINT_HEIGHT / (2.0 * NORMAL_SCALE_FACTOR)
	);

	const Point paint_min(4, 1.5);

	Point paint_max;

	QVector<Point> _rhomb(4);
	for (int i = 0; i != 4; ++i) {
		_rhomb[i] = transform.apply(points[i]);
		paint_max.x = qMax(qAbs(_rhomb[i].x), paint_max.x);
		paint_max.y = qMax(qAbs(_rhomb[i].y), paint_max.y);
	}

	if (paint_max.x > paint_size.x || paint_max.y > paint_size.y) {
		compress *= 1.1 * qMax(paint_max.x / paint_size.x, paint_max.y / paint_size.y);
	}
	else if (paint_max.x < paint_min.x && paint_max.y < paint_min.y) {
		compress /= 1.1 * qMax(paint_min.x / paint_max.x, paint_min.y / paint_max.y);
	}

	QVector<QPointF> rhomb(4);
	for (int i = 0; i != 4; ++i) {
		rhomb[i] = coord(_rhomb[i]);
	}

	QVector<QPointF> limacon(n);
	for (int i = 0; i != n; ++i) {
		limacon[i] = coord(transform.apply(points[i + 4]));
	}

	int m = PAINT_HEIGHT * compress / (2 * NORMAL_SCALE_FACTOR);
	for (int i = -m; i != m + 1; ++i) {
		painter.setPen(choosePen(i));
		painter.drawLine(QPointF(0, y_coord(i)), QPointF(PAINT_WIDTH, y_coord(i)));
	}
	m = PAINT_WIDTH * compress / (2 * NORMAL_SCALE_FACTOR);
	for (int i = -m; i != m + 1; ++i) {
		painter.setPen(choosePen(i));
		painter.drawLine(QPointF(x_coord(i), 0), QPointF(x_coord(i), PAINT_HEIGHT));
	}

	QPainterPath rhombPath;
	rhombPath.addPolygon(QPolygonF(rhomb));
	painter.setPen(QPen(Qt::black, 2));
	painter.setBrush(Qt::BDiagPattern);

	QPainterPath limaconPath;
	limaconPath.setFillRule(Qt::WindingFill);
	limaconPath.addPolygon(QPolygonF(limacon));

	QPainterPath figurePath = rhombPath.subtracted(limaconPath);
	painter.drawPath(figurePath);

	painter.setBrush(Qt::NoBrush);
	painter.drawPath(limaconPath);
	painter.drawPoint(rhomb[0]);
}
