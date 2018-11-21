#ifndef SIGNALSUPPRESSOR_H
#define SIGNALSUPPRESSOR_H

#include <QObject>
#include <QVariant>
#include <QMutex>

class SignalSuppressor : public QObject
{
    Q_OBJECT
public:
    explicit SignalSuppressor(QObject *parent = nullptr);
    void performOperation(); // pass fn pointer?
    QVariant retrieveNewData(QVariant oldData);

signals:
    void suppressedSignal(SignalSuppressor *ptr);

public slots:
    void receiveNewData(QVariant newData);

private:
    QVariant data_m; //consider ringbuffer 4 performance?
    QMutex mutex; //this NEEDS implemented internally as 2 threads will be calling the same file
};

#endif // SIGNALSUPPRESSOR_H
