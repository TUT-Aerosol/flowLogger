#include "mainwindow.h"
#include "ui_mainwindow.h"

#define MAX_PLOT_ITEMS 12*3600

#define flowCurve 0
#define pressureCurve 1
#define deltaPCurve 2
#define tempCurve 3

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Add custom statusbar (otherwise text disappears when using menus):
    statusLbl = new QLabel();
    statusLbl->setAlignment(Qt::AlignTop);
    // Add the statusbar inside ScrollArea to prevent it from resizing the window
    // in case of long message.
    QScrollArea *lblScrollArea = new QScrollArea();
    lblScrollArea->setAlignment(Qt::AlignTop);
    lblScrollArea->setWidgetResizable(true);
    lblScrollArea->setWidget(statusLbl);
    lblScrollArea->setMaximumHeight(15);
    lblScrollArea->verticalScrollBar()->setEnabled(false);
    lblScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    lblScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    lblScrollArea->setFrameShape(QFrame::NoFrame);
    ui->horizontalLayout_2->addWidget(lblScrollArea);

    // Load and show OQ logo:
    QGraphicsScene *scene = new QGraphicsScene();
    QGraphicsSvgItem *svgItem = new QGraphicsSvgItem(":/images/OQlogo.svg");
    svgItem->setScale(0.8);
    scene->addItem(svgItem);
    ui->graphicsView->setStyleSheet("background: transparent");
    ui->graphicsView->fitInView(svgItem->boundingRect(),Qt::KeepAspectRatio);
    qDebug() << "Bounding rect: " << svgItem->boundingRect();
    ui->graphicsView->setScene(scene);
    //ui->graphicsView->fitInView(svgItem->boundingRect(),Qt::KeepAspectRatio);
    ui->graphicsView->show();

    Log("Program started.");

    serialTimer = new QTimer;
    serialTimer->setSingleShot(true);
    serialTimer->setInterval(10000); // 10 sec timeout for serial connection
    connect(serialTimer,SIGNAL(timeout()),this,SLOT(noCommunication()));

    Plot = new OQPlot(ui->qwtPlot);
    QColor Colors[8] = {Qt::blue, Qt::red, Qt::green, Qt::cyan, Qt::magenta, Qt::darkCyan, Qt::gray, Qt::darkBlue};
    Plot->AddCurve("Flow", "slpm", Colors[0],QwtPlot::yLeft,1);
    // The following ones are just placeholders for other possible left Y-axis curves:
    Plot->AddCurve("Pressure","kPa",Colors[0],QwtPlot::yLeft,1);
    Plot->AddCurve("deltaP","Pa",Colors[0],QwtPlot::yLeft,1);
    Plot->AddCurve("Temperature","°C",Colors[0],QwtPlot::yLeft,1);
    // The following ones are just placeholders for possible right Y-axis curves:
    Plot->AddCurve("Flow", "slpm", Colors[1], QwtPlot::yRight,1);
    Plot->AddCurve("Pressure","kPa",Colors[1], QwtPlot::yRight,1);
    Plot->AddCurve("deltaP","Pa",Colors[1], QwtPlot::yRight,1);
    Plot->AddCurve("Temperature","°C",Colors[1], QwtPlot::yRight,1);

    // Hide all curves except the first one:
    for(int i=1; i < 8; i++)
        Plot->HideCurve(i);

    // Check available serial ports:
    comPortMenu = ui->menuSelect_port;
    refreshSerialPorts();
}

void MainWindow::handleData(QString receivedData) {
    if(serialTimer->isActive())
        serialTimer->stop(); // Disable timeout counting, because apparently we are connected...

    /*if(receivedData.back() != '\n' && receivedData.back() != '\r') {
        Log("Incomplete row: " + receivedData);
        serialTimer->start();
        return;
    }*/

    // We have received a complete row. Now lets handle the data.
    receivedData.remove(QChar('\r'));
    receivedData.remove(QChar('\n'));
    //Log("Cleaned up row: " + receivedData);

    QDateTime currentTime = QDateTime::currentDateTime();
    QList<double> currentData;
    QStringList toSaveData;

    if(receivedData.count(',') != 11) {
        Log("Invalid row: " + receivedData);
        for(int i=0; i < 12; i++) {
            currentData.append(NAN);
            toSaveData.append("NaN");
        }
    }
    else {
        QStringList vars = receivedData.split(',');

        for(int i=0; i < vars.length(); i++) {
            bool conversionSuccess;
            double val = vars.at(i).toDouble(&conversionSuccess);
            if(!conversionSuccess) {
                //Log("Conversion not successful.");
                val = NAN;
                toSaveData.append("NaN");
            }
            else
                toSaveData.append(vars.at(i));
            currentData.append(val);
        }
    }

    if(isSaving) {
        if(!saveTimer.isValid()) {
            saveTimer.start();
        }
        if(periodicSavingOn && saveTimer.elapsed() > saveFileNumber*savingPeriod) {
            autoGenerateNewFile();
        }

        // Write all data to file (comma separated):
        out << roundToClosestSecond(currentTime).toString("dd.MM.yyyy hh:mm:ss") << ",";

        for(int i=0; i<toSaveData.length(); i++) {
            if(i > 0) {
                out << ",";
            }
            out << toSaveData.at(i);
        }
        out << "\r\n";
        out.flush();
        saveFile->flush();
        rowsSaved = rowsSaved + 1;
        //ui->RowsSavedTxt->setText(QString("Rows saved: %1").arg(rowsSaved));
    }

    // Check if we exceed the Plot limit:
    if(numDataPoints > MAX_PLOT_ITEMS) {
        for(int i=0; i<Plot->getNumCurves() + 1; i++)
            Plot->RemoveFirstPoint(i);
    }

    // Add the new datapoints to plot:
    Plot->AddPoint(flowCurve,currentTime,currentData.at(0));
    Plot->AddPoint(4+flowCurve,currentTime,currentData.at(0));

    Plot->AddPoint(pressureCurve,currentTime,currentData.at(2));
    Plot->AddPoint(4+pressureCurve,currentTime,currentData.at(2));

    Plot->AddPoint(deltaPCurve,currentTime,currentData.at(1));
    Plot->AddPoint(4+deltaPCurve,currentTime,currentData.at(1));

    Plot->AddPoint(tempCurve,currentTime,currentData.at(3));
    Plot->AddPoint(4+tempCurve,currentTime,currentData.at(3));

    numDataPoints ++;

    serialTimer->start(); // Start the timeout again.
}

void MainWindow::refreshSerialPorts() {
    // First remove all serial ports from COM port menu, so that we can add the currently available ports later:
    QList<QAction*> actionsList = comPortMenu->actions();
    for(int i=0; i<actionsList.length(); i++){
        comPortMenu->removeAction(actionsList.at(i));
    }

    // Refresh serial ports and add them
    serialPorts = QSerialPortInfo::availablePorts();
    int i = 0;
    foreach(const QSerialPortInfo &serialPortInfo, serialPorts) {
        QString portName = serialPortInfo.portName();
        comPortMenu->addAction(portName,this,[this,portName,i]() {connectToPort(portName,i);});
        i ++;
    }
}

void MainWindow::connectToPort(QString portName, int index) {
    // Communication = NONE;
    // Need to operate the serial port in another thread. Otherwise the data is not received
    // when e.g. window is being resized.
    serialThread = new QThread;
    serialPortReader = new OqSerialPort(portName,115200);
    serialPortReader->moveToThread(serialThread);

    connect(serialThread, &QThread::finished, serialThread, &QThread::deleteLater);
    connect(serialThread, &QThread::finished, serialPortReader, &OqSerialPort::deleteLater);
    connect(serialThread, &QThread::started, serialPortReader, &OqSerialPort::init);
    connect(serialPortReader, &OqSerialPort::finished, serialThread, &QThread::quit);
    connect(serialPortReader, &OqSerialPort::finished, serialThread, &QThread::deleteLater);
    connect(serialPortReader, SIGNAL(openStatus(bool)), this, SLOT(handlePortOpening(bool)));
    connect(serialPortReader, SIGNAL(errorOccurred(QSerialPort::SerialPortError)), this, SLOT(handleSerialError(QSerialPort::SerialPortError)));

    serialThread->start();

    connect(serialPortReader, SIGNAL(newData(QString)), this, SLOT(handleData(QString)));
    QList<QAction*> actionsList = comPortMenu->actions();
    for(int i=0; i<actionsList.length(); i++){
        actionsList.at(i)->setEnabled(false);
        if(i == index) {
            actionsList.at(i)->setText(portName + " (active)");
        }
    }
}

void MainWindow::handlePortOpening(bool isOpen) {
    if(isOpen) {
        isPortOpen = true;
        serialTimer->start(); // Start timer to disconnect after 10 secs if no data received
        Log("You connected to port " + serialPortReader->getPortName());
        serialPortReader->write("t");
    }
    else {
        serialTimer->stop();
        isPortOpen = false;
        refreshSerialPorts();
        QMessageBox::information(this, tr("COM port error"),"Could not open COM port.");
        isPortOpen = false;
    }
}

void MainWindow::handleSerialError(QSerialPort::SerialPortError error) {
    switch(error) {
    case QSerialPort::NoError:
        break;
    case QSerialPort::DeviceNotFoundError:
        closeSerialPort();
        Log("COM port not found.");
        QMessageBox::information(this, tr("COM port error"),"COM port not found.");
        break;
    case QSerialPort::PermissionError:
        closeSerialPort();
        Log("COM port permission error (in use?)");
        QMessageBox::information(this, tr("COM port error"),"COM port permission error (in use?).");
        break;
    case QSerialPort::OpenError:
        closeSerialPort();
        Log("Attempting to open an already opened port.");
        QMessageBox::information(this, tr("COM port error"),"Attempting to open an already opened port.");
        break;
    case QSerialPort::NotOpenError:
        closeSerialPort();
        Log("COM port is not open.");
        QMessageBox::information(this, tr("COM port error"),"Port is not open.");
        break;
    case QSerialPort::WriteError:
        Log("Serial port I/O error while writing data.");
        break;
    case QSerialPort::ReadError:
        Log("Serial port I/O error while reading data.");
        break;
    case QSerialPort::ResourceError:
        closeSerialPort();
        Log("COM port unavailable (cable plugged out?).");
        QMessageBox::information(this, tr("COM port error"),"COM port unavailable (cable plugged out?).");
        break;
    case QSerialPort::TimeoutError:
        Log("Serial port time out error.");
        break;
    default:
        Log("Unknown serial port error.");
        break;
    }
}

void MainWindow::closeSerialPort() {
    if(serialTimer->isActive())
        serialTimer->stop();
    if(isPortOpen)
        serialPortReader->close();
    isPortOpen = false;

    refreshSerialPorts();
}

void MainWindow::noCommunication() {
    Log("No response from the COM port.");
    serialTimer->stop();
    isPortOpen = false;
    serialPortReader->close();
    QList<QAction*> actionsList = comPortMenu->actions();
    for(int i=0; i<actionsList.length(); i++){
        actionsList.at(i)->setEnabled(true);
        QString actName = actionsList.at(i)->text();
        if(actName.contains(" (active)")) {
            actName = actName.remove(" (active)");
            actionsList.at(i)->setText(actName);
        }
    }
    QMessageBox::information(this, tr("COM port error"),"No response from the COM port. Disconnecting.");
}

void MainWindow::Log(QString msg) {
    QString fullMsg = QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss") + ": " + msg;
    qDebug() << fullMsg;

    statusLbl->setText(fullMsg);
    allLogs.append(fullMsg);
    if(isLogSaving) {
        logOut << fullMsg << "\r\n";
        logOut.flush();
        logSaveFile->flush();
    }
    emit logTextAdded(fullMsg);
}

void MainWindow::printHeadersToFile() {
    out << "[FILEVERSION]\r\n";
    out << "1.0\r\n";

    // Write headers

    out << "\r\n[DATA]\r\n";
    out << "Time (Y-m-d H:M:S),Flow (slpm),dP (Pa),P_abs (kPa),T (°C),T_dP (°C),T_p_abs (°C),dP_raw,T_raw,dP_raw_0,T_raw_0,k,b";
    out << "\r\n";
    out.flush();
    saveFile->flush();
}

void MainWindow::startSaving() {
    if(isSaving) {
        isSaving = false;
        saveTimer.invalidate();
        saveFile->flush();
        saveFile->close();
        periodicSavingOn = false;
        ui->actionSave_as->setText(tr("Save as..."));
        rowsSaved = 0;
        ui->saveLabel->setText("NOT saving!");
        // Stop saving log:
        if(isLogSaving) {
            isLogSaving = false;
            logOut.flush();
            logSaveFile->flush();
            logSaveFile->close();
            ui->actionSave_log_as->setEnabled(false);
            ui->actionSave_log_as->setText("Save log as...");
        }
        return;
    }

    const QString DEFAULT_DIR_KEY("defaultSaveDir");
    QSettings appSettings;
    QString defaultDirectory;

    if(appSettings.contains(DEFAULT_DIR_KEY)) {
        defaultDirectory = appSettings.value(DEFAULT_DIR_KEY).toString();
        QDir testDir(defaultDirectory);
        if(!testDir.exists()) {
            qDebug() << "Default directory does not exist.";
            qDebug() << "Default directory is: " << defaultDirectory;
            defaultDirectory = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
        }
    }
    else {
        defaultDirectory = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
        qDebug() << "No default directory found.";
        qDebug() << "Using " << defaultDirectory << " as the directory";
    }

    QDateTime currentTime = QDateTime::currentDateTime();
    QString currentTimeStr = currentTime.toString("yyyyMMdd_hhmm");
    QString defaultFilename = currentTimeStr + defaultFileSuffix + ".dat";

    OQFileDialog *fileDialog = new OQFileDialog(this, "Save as...",{"Data file (*.dat)","All files (*)"},defaultFilename,true);
    fileDialog->setAcceptMode(QFileDialog::AcceptSave);
    if(!defaultDirectory.isEmpty()) {
        fileDialog->setDirectory(defaultDirectory);
    }
    fileDialog->setFileMode(QFileDialog::AnyFile);
    if(!fileDialog->exec()) {
        return;
    }

    QString selectedFile = fileDialog->selectedFiles().first();

    if(!selectedFile.isEmpty()) {
        QFileInfo fi(selectedFile);
        appSettings.setValue(DEFAULT_DIR_KEY,fi.absolutePath());

        saveFile = new QFile(selectedFile);
        originalSaveFile = new QFile(selectedFile);

        if(!saveFile->open(QIODevice::WriteOnly)) {
            QMessageBox::information(this, tr("Unable to open file"),saveFile->errorString());
            return;
        }

        out.setDevice(saveFile);

        printHeadersToFile();

        isSaving = true;
        ui->actionSave_as->setText(tr("Stop saving"));
        ui->actionSave_log_as->setEnabled(true);
        Log("Data saving started.");

        periodicSavingOn = false;
        if(fileDialog->periodCheckBox->isChecked()) {
            bool ok;
            savingPeriod = (qint64) (fileDialog->periodTextEdit->text().toDouble(&ok) * 3600 * 1000); // In msecs
            if(!ok || savingPeriod < 60*1000) {
                periodicSavingOn = false;
                // Show dialog
                QMessageBox wrnBox;
                wrnBox.setText("Could not resolve saving period. Automatic file generation is off.");
                wrnBox.setStandardButtons(QMessageBox::Ok);
                wrnBox.exec();
                return;
            }
            else {
                periodicSavingOn = true;
                saveFileNumber = 1;
            }
        }
        rowsSaved = 0;

        ui->saveLabel->setText("Saving on.");
    }
}

void MainWindow::autoGenerateNewFile() {
    QFileInfo oldFileInfo = QFileInfo(*originalSaveFile);
    QString oldBaseName = oldFileInfo.baseName();
    QString oldSuff = oldFileInfo.completeSuffix();
    QString oldPath = oldFileInfo.absolutePath();

    QDateTime currentTime = QDateTime::currentDateTime();
    QString currentTimeStr = currentTime.toString("yyyyMMdd_hhmm");

    QString newFileName = oldPath + QDir::separator() + oldBaseName + "_" + currentTimeStr + "." + oldSuff;

    int additionalSuffix = 0;
    while(fileExists(newFileName) && additionalSuffix < 100) {
        // If file already exists, let's add a number (0-99) and try with that:
        newFileName = oldPath + QDir::separator() + oldBaseName + "_" + currentTimeStr + "_" + QString::number(additionalSuffix) + "." + oldSuff;
        additionalSuffix ++;
    }

    if(additionalSuffix < 100) {
        out.flush();
        saveFile->flush();
        saveFile->close();
        saveFile = new QFile(newFileName);
        if(!saveFile->open(QIODevice::WriteOnly)) {
            QMessageBox::information(this, tr("Unable to open save file"),saveFile->errorString());
            return;
        }
        out.setDevice(saveFile);
        printHeadersToFile();
    }
    else {
        Log("Could not create new save file; continuing in the current file.");
        return;
    }
    Log("New save file started (" + oldBaseName + "_" + currentTimeStr + "_" + QString::number(additionalSuffix-1) + "." + oldSuff + ")");
    saveFileNumber ++;
}

bool MainWindow::fileExists(QString path) {
    return QFileInfo::exists(path) && QFileInfo(path).isFile();
}

QDateTime MainWindow::roundToClosestSecond(QDateTime datTimToRound) {
    int msecs = datTimToRound.time().msec();
    int secondsToAdd = 0;
    if(msecs >= 500) {
        secondsToAdd = 1;
    }

    QDateTime timeCopy = datTimToRound.addSecs(secondsToAdd);
    timeCopy = timeCopy.addMSecs(-msecs); // msecs -> 0

    return timeCopy;
}


MainWindow::~MainWindow()
{
    if(isPortOpen) {
        serialPortReader->close();
    }
    if(isSaving) {
        isSaving = false;
        saveFile->flush();
        saveFile->close();
    }
    delete Plot;
    delete ui;
}


void MainWindow::on_rbNoneLeft_clicked()
{
    for(int i=0; i < 4; i++) {
        Plot->HideCurve(i);
    }
}




void MainWindow::on_rbFlowLeft_clicked()
{
    for(int i=0; i < 4; i++) {
        Plot->HideCurve(i);
    }
    Plot->ShowCurve(flowCurve);
}


void MainWindow::on_rbPressureLeft_clicked()
{
    for(int i=0; i < 4; i++) {
        Plot->HideCurve(i);
    }
    Plot->ShowCurve(pressureCurve);
}


void MainWindow::on_rbDpLeft_clicked()
{
    for(int i=0; i < 4; i++) {
        Plot->HideCurve(i);
    }
    Plot->ShowCurve(deltaPCurve);
}


void MainWindow::on_rbTempLeft_clicked()
{
    for(int i=0; i < 4; i++) {
        Plot->HideCurve(i);
    }
    Plot->ShowCurve(tempCurve);
}


void MainWindow::on_rbNoneRight_clicked()
{
    for(int i=4; i < 8; i++) {
        Plot->HideCurve(i);
    }
}


void MainWindow::on_rbFlowRight_clicked()
{
    for(int i=4; i < 8; i++) {
        Plot->HideCurve(i);
    }
    Plot->ShowCurve(4+flowCurve);
}


void MainWindow::on_rbPressureRight_clicked()
{
    for(int i=4; i < 8; i++) {
        Plot->HideCurve(i);
    }
    Plot->ShowCurve(4+pressureCurve);
}



void MainWindow::on_rbDpRight_clicked()
{
    for(int i=4; i < 8; i++) {
        Plot->HideCurve(i);
    }
    Plot->ShowCurve(4+deltaPCurve);
}


void MainWindow::on_rbTempRight_clicked()
{
    for(int i=4; i < 8; i++) {
        Plot->HideCurve(i);
    }
    Plot->ShowCurve(4+tempCurve);
}


void MainWindow::on_actionDisconnect_triggered()
{
    closeSerialPort();
}


void MainWindow::on_actionSave_as_triggered()
{
    startSaving();
}


void MainWindow::on_actionRefresh_ports_triggered()
{
    refreshSerialPorts();
}


void MainWindow::on_actionAbout_triggered()
{
    aboutWindow *aboutwindow = new aboutWindow(this);
    aboutwindow->setWindowFlags(Qt::Window);
    aboutwindow->show();
}


void MainWindow::on_actionQuick_guide_triggered()
{
    quickguidewindow *guideWindow = new quickguidewindow(this);
    guideWindow->setWindowFlags(Qt::Window);
    guideWindow->show();
}


void MainWindow::on_actionShow_log_triggered()
{
    logwindow *logWindow = new logwindow(this);
    logWindow->refreshTextBox(&allLogs);
    logWindow->setWindowFlags(Qt::Window);
    logWindow->show();
    connect(this,SIGNAL(logTextAdded(QString)),logWindow,SLOT(addLogText(QString)));
}


void MainWindow::on_actionSave_log_as_triggered()
{
    if(isLogSaving) {
        isLogSaving = false;
        logOut.flush();
        logSaveFile->flush();
        logSaveFile->close();
        ui->actionSave_log_as->setText("Save log as...");
        return;
    }
    QFileInfo currentFileInfo = QFileInfo(*saveFile);
    QString defaultLogFilename = currentFileInfo.baseName() + ".log";
    QString defaultDirectory = currentFileInfo.absolutePath();

    OQFileDialog *fileDialog = new OQFileDialog(this, "Save log as...",{"Log file (*.log)","All files (*)"},defaultLogFilename,false);
    fileDialog->setAcceptMode(QFileDialog::AcceptSave);
    fileDialog->setDirectory(defaultDirectory);
    fileDialog->setFileMode(QFileDialog::AnyFile);
    if(!fileDialog->exec()) {
        return;
    }

    QString selectedFile = fileDialog->selectedFiles().first();
    if(!selectedFile.isEmpty()) {
        logSaveFile = new QFile(selectedFile);

        if(!logSaveFile->open(QIODevice::WriteOnly)) {
            QMessageBox::information(this, tr("Unable to open file"),logSaveFile->errorString());
            return;
        }
        logOut.setDevice(logSaveFile);

        // Write all logs that already exist:
        for(int i=0; i < allLogs.length(); i++) {
            logOut << allLogs.at(i) << "\r\n";
        }

        logOut.flush();
        logSaveFile->flush();
        isLogSaving = true;
        ui->actionSave_log_as->setText("Stop saving log");
    }
}

