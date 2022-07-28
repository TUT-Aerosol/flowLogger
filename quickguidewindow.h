#ifndef QUICKGUIDEWINDOW_H
#define QUICKGUIDEWINDOW_H

#include <QWidget>

namespace Ui {
class quickguidewindow;
}

class quickguidewindow : public QWidget
{
    Q_OBJECT

public:
    explicit quickguidewindow(QWidget *parent = nullptr);
    ~quickguidewindow();

private:
    Ui::quickguidewindow *ui;
};

#endif // QUICKGUIDEWINDOW_H
