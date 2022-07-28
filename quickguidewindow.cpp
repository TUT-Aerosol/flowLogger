#include "quickguidewindow.h"
#include "ui_quickguidewindow.h"

quickguidewindow::quickguidewindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::quickguidewindow)
{
    ui->setupUi(this);
}

quickguidewindow::~quickguidewindow()
{
    delete ui;
}
