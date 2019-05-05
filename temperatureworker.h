#ifndef TEMPERATUREWORKER_H
#define TEMPERATUREWORKER_H

#include <QObject>
#include <opencv2/core.hpp>
class QMutex;
class QString;
class SignalSuppressor;

class TemperatureWorker : public QObject
{
    Q_OBJECT
public:
    explicit TemperatureWorker(const cv::Mat *masterImage = nullptr, cv::Mat *previewImage = nullptr, QMutex *mutex = nullptr, QObject *parent = nullptr);
    ~TemperatureWorker();

signals:
    void updateDisplayedImage();
    void updateStatus(QString);

public slots:
    void receiveSuppressedSignal(SignalSuppressor *dataContainer);
    void receiveImageAddresses(const cv::Mat *masterImage, cv::Mat *previewImage);

private:
    void performLampTemperatureShift(int parameter);
    int data_m;
    QMutex *mutex_m;
    const cv::Mat *masterImage_m;
    cv::Mat *previewImage_m;
    cv::UMat implicitOclImage_m;
    std::vector <cv::UMat> splitChannelsTmp_m;
};

#endif // TEMPERATUREWORKER_H
