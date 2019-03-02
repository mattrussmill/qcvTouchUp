#ifndef ADJUSTWORKER_H
#define ADJUSTWORKER_H

#include <QObject>
#include <QByteArray>
#include <opencv2/core/core.hpp>
class QMutex;
class SignalSuppressor;

class AdjustWorker : public QObject
{
    Q_OBJECT
public:
    explicit AdjustWorker(QMutex *mutex = nullptr, QObject *parent = nullptr);
    ~AdjustWorker();

signals:
    void updateDisplayedImage();

public slots:
    void receiveSuppressedSignal(SignalSuppressor *dataContainer);
    void receiveImageAddresses(const cv::Mat *masterImage, cv::Mat *previewImage);

private:
    void performImageAdjustments(float *parameter);
    QByteArray data;
    QMutex *mutex_m;
    const cv::Mat *masterImage_m;
    cv::Mat *previewImage_m;
    cv::Mat lookUpTable_m;
    std::vector <cv::Mat> splitChannelsTmp_m;
};

#endif // ADJUSTWORKER_H
