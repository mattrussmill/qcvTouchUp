#include "adjustworker.h"
#include <QDebug>

AdjustWorker::AdjustWorker(QObject *parent) : QObject(parent)
{
    qDebug() << "AdjustWorker created";

}

AdjustWorker::~AdjustWorker()
{
    qDebug() << "AdjustWorker destroyed";
}
