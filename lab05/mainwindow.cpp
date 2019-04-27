#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QPainter>
#include <QLabel>

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow),
	closed(false),
	bitmap(720, 720),
	image(720, 720, QImage::Format_Mono),
	scene(new QGraphicsScene(0, 0, 720, 720))
{
	ui->setupUi(this);

	bitmap_item = scene->addPixmap(bitmap);
	ui->graphicsView->setScene(scene);

	clearImage();
	displayImage();
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
	if (event->x() < 240 || event->y() < 10)
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
	default:
		break;
	}

	addPoint(QPoint(event->x() - 240, event->y() - 10), drawType);
}

void MainWindow::on_addPointPushButton_clicked()
{
	const int x = ui->xSpinBox->text().toInt();
	const int y = ui->ySpinBox->text().toInt();
	addPoint(QPoint(x, y), DrawType::none);
}

void MainWindow::on_closePushButton_clicked()
{
	if (edges.size() <= 2) {
		QMessageBox::critical(this, "Error", "Need at least 2 edges");
		return;
	}

	if (closed) {
		QMessageBox::critical(this, "Error", "Already closed");
		return;
	}

	addEdge(QLine(points[points.size() - 1], points[0]));
	closed = true;
}

// Алгоритм заполнения с упорядоченным списком ребер Используя изложенный идеи,
// можно предложить алгоритмы растровой развертки сплошных областей, называемых
// алгоритмами с упорядоченным списком ребер. Эти алгоритмы отличаются друг от
// друга способами сортировки точек пересечения сканирующих строк с ребрами
// многоугольника. Эффективность сортировки определяет эффективность
// рассматриваемых алгоритмов.
// Рассмотрим простой алгоритм с упорядоченным списком ребер. На первом этапе
// производится подготовка исходных данных. Этот этап предусматривает
// определение точек пересечения ребер многоугольника со сканирующими строками
// (рис.3.5.3). Для решения этой задачи можно использовать алгоритм ЦДА или
// алгоритм Брезенхема. Горизонтальные ребра не рассматриваются. Найденные
// точки сортируются: сначала производится сортировка по строкам, а затем для
// точек в пределах одной строки производится сортировка по возрастанию
// координаты x. Например, точка (x1, y1) предшествует точке (x2, y2), если
// y1 > y2 или y1 = y2 и x1 ≤ x2.
// На втором этапе подготовленные данные преобразуются в растровую форму: в
// отсортированном списке выделяются пары точек (x1, y1) и (x2, y2). Структура
// списка гарантирует, что y = y1 = y2 и x1 ≤ x2. Закрашиваются пикселы, лежащие
// на данной сканирующей строке в интервале x1 ≤ x ≤ x2.

// БОЛЕЕ ЭФФЕКТИВНЫЕ АЛГОРИТМЫ С УПОРЯДОЧЕННЫМ СПИСКОМ РЕБЕР
// В предыдущем алгоритме формируется большой список точек пересечения, который
// затем необходимо полностью отсортировать. Эффективность алгоритма существенно
// возрастает, если повысить эффективность сортировки. Эта задача решается путем
// разделения сортировки по координате y и сортировки в строке по возрастанию
// координаты x на основе групповой сортировки по y. Алгоритм в этом случае
// выглядит следующим образом:
// Подготовка исходных данных.
//     1. Определение для каждого ребра многоугольника точек пересечения со
//        сканирующими строками. Для решения этой задачи используется алгоритм
//        Брезенхема или ЦДА. Горизонтальные ребра не рассматриваются.
//     2. Размещение координат x найденных точек пересечения в группе,
//        соответствующей y координате сканирующей строки.
//     3. Сортировка для каждой y-группы координат x точек пересечения в порядке
//        возрастания, т.е. x1 предшествует x2, если x1 ≤ x2.
// Преобразование данных в растровую форму.
//     1. Выделение для каждой сканирующей строки из списка координат x точек
//        пересечения пар точек пересечений. Закраска пикселов, имеющих
//        x-координаты, лежащие в интервале x1 ≤ x ≤ x2.
//
// В данном алгоритме сортировки по координате x и координате y разделены,
// поэтому растровую развертку можно производить, не дожидаясь завершения всего
// процесса сортировки. В данном алгоритме легче также добавлять или удалять
// информацию из дисплейного списка. Необходимо только добавить или удалить
// информацию из соответствующей y-группы, следовательно, заново сортировать
// требуется только эти y-группы.
// Хотя процедура сортировки упрощается, однако и в этом алгоритме имеются
// недостатки. Они связаны либо с ограниченным числом пересечений каждой
// сканирующей строки с ребрами многоугольника, либо с необходимостью
// резервирования большого количества памяти, существенная доля которой не будет
// использоваться. Перечисленные недостатки преодолеваются благодаря
// использованию связного списка, т.е. путем введения добавочной структуры
// данных в виде списка активных ребер, как это предлагалось для растровой
// развертки в реальном времени.
//
// В итоге алгоритм с упорядоченным списком ребер, использующий список активных
// ребер, выглядит следующим образом:
// Подготовка исходных данных.
//     1. Определение для каждого ребра многоугольника наивысшей сканирующей
//        строки, пересекаемой ребром.
//     2. Занесение ребра многоугольника в y-группу, соответствующую этой
//        сканирующей строки.
//     3. Сохранение в связном списке следующих значений: начального значения
//        координаты x точек пересечения; Δy - числа сканирующих строк,
//        пересекаемых ребром многоугольника; Δx - шага изменения координаты x
//        при переходе от одной сканирующей строки к следующей строке.
// Преобразование этих данных в растровую форму.
//     1. Проверка для каждой сканирующей строки y-группы на наличие новых
//        ребер. Добавление новых ребер в САР.
//     2. Сортировка x-координат точек пересечения из САР в порядке возрастания,
//        т.е. x1 предшествует x2, если x1 ≤ x2.
//     3. Выделение пар точек пересечений в отсортированном списке.
//     4. Закрашивание пикселов на очередной сканирующей строке со значениями
//        x-координаты, лежащей в интервале x1 ≤ x ≤ x2.
//     5. Уменьшение на единицу количества пересекаемых строк для всех ребер из
//        САР: Δy = Δy - 1. Исключение ребра из САР при Δy<0. Вычисление нового
//        значения координаты x точки пересечения со сканирующей строкой
//        x = x + Δx.
//     6. Переход к следующей сканирующей строке.


void MainWindow::on_fillPushButton_clicked()
{
	if (!closed) {
		QMessageBox::critical(this, "Error", "Figure is not closed");
		return;
	}

	QPainter painter(&bitmap);

//	for () {
//		for () {
//			if (ui->delayCheckBox->isChecked()) {
//				repaint();
//				displayImage();
//			}
//		}
//	}


	displayImage();
}

void MainWindow::on_clearPushButton_clicked()
{
	clearImage();
	closed = false;
	points.clear();
	edges.clear();
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

	QPainter painter(&bitmap);
	painter.drawLine(edge);
	image = bitmap.toImage();

	displayImage();
}

void MainWindow::clearImage()
{
	image.fill(0);
	bitmap.fill();
	displayImage();
}

void MainWindow::displayImage()
{
	bitmap_item->setPixmap(bitmap);
}
