#include "oqfiledialog.h"

#include <QLabel>
#include <QSizePolicy>
#include <qgridlayout.h>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>

OQFileDialog::OQFileDialog(QWidget *parent, QString title, QStringList filters, QString defaultFilename, bool addPeriodicCheckbox) : QFileDialog(parent)
{
    setOption(QFileDialog::DontUseNativeDialog);
    if(addPeriodicCheckbox) {
        QGridLayout *mainLayout = dynamic_cast <QGridLayout*>(this->layout());
        QHBoxLayout *extraLayout = new QHBoxLayout(0);
        extraLayout->setSpacing(0);
        periodCheckBox = new QCheckBox("Create new file every");
        periodCheckBox->setCheckState(Qt::CheckState::Checked);
        periodCheckBox->adjustSize();
        periodTextEdit = new QLineEdit("12");
        periodTextEdit->setFixedWidth(30);
        periodTextEdit->setAlignment(Qt::AlignmentFlag::AlignLeft);
        QLabel *hoursText = new QLabel(" hours.");
        extraLayout->addWidget(periodCheckBox);
        extraLayout->addWidget(periodTextEdit);
        extraLayout->addWidget(hoursText);
        extraLayout->addStretch(1);
        int numRows = mainLayout->rowCount();
        mainLayout->addLayout(extraLayout,numRows,1);
    }

    //QList<QUrl> newSidebarUrls = this->sidebarUrls();
    QList<QUrl> newSidebarUrls;

    // Add "This PC":
    newSidebarUrls.append(QUrl("file:"));


    // Let's check if Desktop/OQ_data exists (or similar):
    QStringList dataPaths = {"OQ_data","oq_data","Oq_data","OQdata","oqData","Data","data"};
    for(int i=0; i<dataPaths.length(); i++) {
        QString OqDataPath = QDir::fromNativeSeparators(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) + "\\" + dataPaths.at(i));
        QDir *OqDataDir = new QDir(OqDataPath);
        if(OqDataDir->exists()) {
            newSidebarUrls.append(QUrl::fromLocalFile(OqDataPath));
        }
    }

    // Add "Desktop" and "Documents"
    newSidebarUrls.append(QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation)));
    newSidebarUrls.append(QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)));


    qDebug() << "New sidebar urls: ";
    for(int i=0; i<newSidebarUrls.length(); i++){
        qDebug() << newSidebarUrls.at(i).toString();
    }



    this->setSidebarUrls(newSidebarUrls);

    this->setWindowTitle(title);
    this->selectFile(defaultFilename);
    this->setNameFilters(filters);
}
