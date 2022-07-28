#include "oqserialport.h"
#include <QDebug>

OqSerialPort::OqSerialPort(QString portName, qint32 baudRate, QObject *parent) :
    QObject(parent)
{
    m_serialPort = new QSerialPort;
    m_portName = portName;
    m_baudRate = baudRate;
}

void OqSerialPort::init() {
    qRegisterMetaType<QSerialPort::SerialPortError>();
    connect(m_serialPort, &QSerialPort::readyRead, this, &OqSerialPort::handleReadyRead);
    connect(m_serialPort, &QSerialPort::errorOccurred, this, &OqSerialPort::handleError);
    m_serialPort->setPortName(m_portName);
    m_serialPort->setBaudRate(m_baudRate);

    bool status = m_serialPort->open(QIODevice::ReadWrite);
    Q_EMIT(openStatus(status));
    qDebug() << "Serial initialization finished.";
}

void OqSerialPort::write(const char *dataToWrite) {
    m_serialPort->write(dataToWrite);
}

void OqSerialPort::close() {
    if(m_serialPort->isOpen())
        m_serialPort->close();
    qDebug() << "Serial closed.";
    qDebug() << "Serial finished";
    Q_EMIT(finished());
}

bool OqSerialPort::isOpen() {
    return m_serialPort->isOpen();
}

QString OqSerialPort::getPortName() {
    return m_serialPort->portName();
}

void OqSerialPort::handleReadyRead() {
    //qDebug() << "Ready read";
    m_readData.append(m_serialPort->readAll());
    QStringList rows = m_readData.split("\n");
    if(rows.last().endsWith("\n"))
        m_readData.clear();
    else {
        m_readData = rows.last();
        rows.removeLast();
    }
    for(int i=0; i < rows.length(); i ++) {
        //qDebug() << "Row " << i << ": " << rows.at(i);
        QString textToEmit = rows.at(i);
        Q_EMIT(newData(textToEmit));
        // qDebug() << "This line executed after QEMIT";
    }
    /*if(m_readData.endsWith("\n")){
        QString textToEmit = m_readData;
        Q_EMIT(newData(textToEmit));
        m_readData.clear();
    }*/
    /*if(m_serialPort->canReadLine()) {
        QString textToEmit = m_serialPort->readLine();
        //m_serialPort->clear(QSerialPort::Input);
        qDebug() << "Line received: " << textToEmit;
        Q_EMIT(newData(textToEmit));
        //m_serialPort->waitForReadyRead(50);
    }*/
}

void OqSerialPort::handleError(QSerialPort::SerialPortError error) {
    //if(m_serialPort->isOpen())
        Q_EMIT(errorOccurred(error));
}
