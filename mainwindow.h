#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QScrollArea>
#include <QScrollBar>
#include <QDateTime>
#include <QFile>
#include <QTimer>
#include <QSerialPortInfo>
#include <QThread>
#include <QMessageBox>
#include <QSettings>
#include <QDir>
#include <QStandardPaths>

#include "oqserialport.h"
#include "oqplot.h"
#include "oqfiledialog.h"
#include "aboutwindow.h"
#include "quickguidewindow.h"
#include "logwindow.h"


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    QLabel *statusLbl;
    QFile *logSaveFile;
    QFile *saveFile;
    QFile *originalSaveFile;
    QTimer *serialTimer;
    OQPlot *Plot;
    QMenu *comPortMenu;
    QThread *serialThread;
    OqSerialPort *serialPortReader;

    QStringList allLogs;
    QTextStream logOut;
    QList<QSerialPortInfo> serialPorts;
    QTextStream out;
    QElapsedTimer saveTimer;
    QString defaultFileSuffix = "Flow";

    bool isLogSaving = false;
    bool isSaving = false;
    bool isPortOpen = false;
    bool periodicSavingOn  = false;
    bool newRowStarted = true;

    qint32 rowsSaved = 0;
    qint32 numDataPoints = 0;
    qint64 savingPeriod;
    qint64 saveFileNumber;

    void Log(QString msg);
    void refreshSerialPorts();
    void closeSerialPort();
    void requestData();
    void printHeadersToFile();
    void startSaving();
    void autoGenerateNewFile();

    bool fileExists(QString path);
    QDateTime roundToClosestSecond(QDateTime datTimToRound);
signals:
    void logTextAdded(QString fullMsg);

public slots:
    void connectToPort(QString portName,int index);
    void handleData(QString receivedData);
    void noCommunication();
    void handlePortOpening(bool isOpened);
    void handleSerialError(QSerialPort::SerialPortError error);

private slots:
    void on_rbNoneLeft_clicked();

    void on_rbFlowLeft_clicked();

    void on_rbPressureLeft_clicked();

    void on_rbDpLeft_clicked();

    void on_rbTempLeft_clicked();

    void on_rbNoneRight_clicked();

    void on_rbFlowRight_clicked();

    void on_rbPressureRight_clicked();

    void on_rbDpRight_clicked();

    void on_rbTempRight_clicked();

    void on_actionDisconnect_triggered();

    void on_actionSave_as_triggered();

    void on_actionRefresh_ports_triggered();

    void on_actionAbout_triggered();

    void on_actionQuick_guide_triggered();

    void on_actionShow_log_triggered();

    void on_actionSave_log_as_triggered();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
