#ifndef OQSERIALPORT_H
#define OQSERIALPORT_H

#include <QObject>
#include <QSerialPort>

class OqSerialPort : public QObject
{
    Q_OBJECT
public:
    explicit OqSerialPort(QString portName, qint32 baudRate, QObject *parent = nullptr);
    void init();
    void write(const char *dataToWrite);
    void close();
    bool isOpen();
    QString getPortName();
private slots:
    void handleReadyRead();
    void handleError(QSerialPort::SerialPortError error);
signals:
    void newData(QString serialData);
    void openStatus(bool status);
    void finished();
    void errorOccurred(QSerialPort::SerialPortError);
private:
    QSerialPort *m_serialPort;
    qint32 m_baudRate;
    QString m_portName;
    //QByteArray m_readData;
    QString m_readData;

};

#endif // OQSERIALPORT_H
