#include "adjustworker.h"
#include <QDebug>

AdjustWorker::AdjustWorker(const cv::Mat *masterImage, cv::Mat *previewImage, QMutex *mutex, QObject *parent) : QObject(parent)
{
    qDebug() << "AdjustWorker created";

}

AdjustWorker::~AdjustWorker()
{
    qDebug() << "AdjustWorker destroyed";
}
