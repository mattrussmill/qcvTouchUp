#ifndef TRANSFORMWORKER_H
#define TRANSFORMWORKER_H

#include <QObject>

class TransformWorker : public QObject
{
    Q_OBJECT
public:
    explicit TransformWorker(QObject *parent = nullptr);

signals:

public slots:
};

#endif // TRANSFORMWORKER_H