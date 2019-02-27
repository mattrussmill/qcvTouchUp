#ifndef ADJUSTWORKER_H
#define ADJUSTWORKER_H

#include <QObject>

class AdjustWorker : public QObject
{
    Q_OBJECT
public:
    explicit AdjustWorker(QObject *parent = nullptr);
    ~AdjustWorker();

signals:

public slots:
};

#endif // ADJUSTWORKER_H
