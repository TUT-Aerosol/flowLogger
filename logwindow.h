#ifndef LOGWINDOW_H
#define LOGWINDOW_H

#include <QWidget>

namespace Ui {
class logwindow;
}

class logwindow : public QWidget
{
    Q_OBJECT

public:
    explicit logwindow(QWidget *parent = nullptr);
    ~logwindow();

    void refreshTextBox(QStringList *logList);
public slots:
    void addLogText(QString newLogTxt);

private slots:
    void on_autoscrollCheckBox_stateChanged(int arg1);

private:
    Ui::logwindow *ui;
};

#endif // LOGWINDOW_H
