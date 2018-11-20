#ifndef SIGNALSUPPRESSOR_H
#define SIGNALSUPPRESSOR_H

#include <QObject>
#include <QStack>
#include <QVariant>
class QPoint;
class QMutex;

class SignalSuppressor : public QObject
{
    Q_OBJECT
public:
    explicit SignalSuppressor(QObject *parent = nullptr);
    ~SignalSuppressor();
    void performOperation(); // pass fn pointer?
    QVariant retrieveNewData(QVariant oldData);
    uint16_t getBufferSize();
    void setBufferSize(uint16_t size);

signals:
    void suppressedSignal(SignalSuppressor *ptr);

public slots:
    void receiveNewData(QVariant newData);

private:
    void deleteOldData(); //must leave one item
    QStack<QVariant> data_m; //consider ringbuffer 4 performance?
    uint16_t maxBuffer_m = 255;
    QMutex mutex; //this NEEDS implemented internally as 2 threads will be calling the same file
};

#endif // SIGNALSUPPRESSOR_H
