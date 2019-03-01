#ifndef ADJUSTWORKER_H
#define ADJUSTWORKER_H

#include <QObject>

class AdjustWorker : public QObject
{
    Q_OBJECT
public:
    explicit AdjustWorker(const cv::Mat *masterImage = nullptr, cv::Mat *previewImage = nullptr, QMutex *mutex = nullptr, QObject *parent = nullptr);
    ~AdjustWorker();

signals:

public slots:
};

#endif // ADJUSTWORKER_H
