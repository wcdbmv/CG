#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
	Q_OBJECT

public:
	explicit Dialog(const QVector<QVector<double>> &ns, int M, QWidget *parent = 0);
	~Dialog();

private:
	Ui::Dialog *ui;
};

#endif // DIALOG_H
