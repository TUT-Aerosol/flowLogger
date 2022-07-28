#include "aboutwindow.h"
#include "ui_aboutwindow.h"

aboutWindow::aboutWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::aboutWindow)
{
    ui->setupUi(this);

    QGraphicsScene *scene = new QGraphicsScene();
    QGraphicsSvgItem *svgItem = new QGraphicsSvgItem(":/images/OQlogo.svg");
    svgItem->setScale(0.5);
    scene->addItem(svgItem);
    ui->graphicsView->setStyleSheet("background: transparent");
    ui->graphicsView->fitInView(svgItem->boundingRect(),Qt::KeepAspectRatio);
    ui->graphicsView->setScene(scene);
    ui->graphicsView->show();
    ui->textEdit->setFixedWidth(ui->textEdit->document()->idealWidth()+
                                     ui->textEdit->contentsMargins().left()+
                                     ui->textEdit->contentsMargins().right());

    ui->versionLabel->setText("Version: " + QString(PROGRAM_VERSION));
}

aboutWindow::~aboutWindow()
{
    delete ui;
}
