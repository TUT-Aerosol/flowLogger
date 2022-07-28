#include "logwindow.h"
#include "ui_logwindow.h"
#include <QScrollBar>

logwindow::logwindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::logwindow)
{
    ui->setupUi(this);
}

logwindow::~logwindow()
{
    delete ui;
}

void logwindow::refreshTextBox(QStringList *logList) {
    for(int i=0; i<logList->length(); i++)
        ui->logPlainTextEdit->insertPlainText(logList->at(i) + "\r\n");
    if(ui->autoscrollCheckBox->checkState() == Qt::Checked)
        ui->logPlainTextEdit->verticalScrollBar()->setValue(ui->logPlainTextEdit->verticalScrollBar()->maximum());
}

void logwindow::addLogText(QString newLogTxt) {
    QTextCursor previousCursor = ui->logPlainTextEdit->textCursor();
    int previousPos = ui->logPlainTextEdit->verticalScrollBar()->value();
    ui->logPlainTextEdit->moveCursor(QTextCursor::End);
    ui->logPlainTextEdit->insertPlainText(newLogTxt + "\r\n");
    ui->logPlainTextEdit->setTextCursor(previousCursor); // Move cursor back to where it was

    if(ui->autoscrollCheckBox->checkState() == Qt::Checked)
        ui->logPlainTextEdit->verticalScrollBar()->setValue(ui->logPlainTextEdit->verticalScrollBar()->maximum());
    else
        ui->logPlainTextEdit->verticalScrollBar()->setValue(previousPos); // Move scrollbar back to where it was
}

void logwindow::on_autoscrollCheckBox_stateChanged(int state)
{
    if(state == Qt::Checked){
        ui->logPlainTextEdit->verticalScrollBar()->setValue(ui->logPlainTextEdit->verticalScrollBar()->maximum());
    }
}
