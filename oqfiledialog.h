#ifndef OQFILEDIALOG_H
#define OQFILEDIALOG_H

#include <QCheckBox>
#include <QFileDialog>
#include <QLineEdit>



class OQFileDialog : public QFileDialog
{
public:
    OQFileDialog(QWidget *parent, QString title, QStringList filters, QString defaultFilename, bool addPeriodicCheckbox);
    QCheckBox *periodCheckBox;
    QLineEdit *periodTextEdit;
};

#endif // OQFILEDIALOG_H
