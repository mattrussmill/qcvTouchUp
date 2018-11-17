#ifndef SIGNALSUPPRESSOR_H
#define SIGNALSUPPRESSOR_H

#include <QObject>

class SignalSuppressor : public QObject
{
    Q_OBJECT
public:
    explicit SignalSuppressor(QObject *parent = nullptr);

signals:

public slots:
};

#endif // SIGNALSUPPRESSOR_H