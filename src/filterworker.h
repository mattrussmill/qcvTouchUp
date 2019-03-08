#ifndef FILTERWORKER_H
#define FILTERWORKER_H

#include <QObject>
#include <QByteArray>
#include <opencv2/core.hpp>
class QMutex;
class QString;
class SignalSuppressor;

class FilterWorker : public QObject
{
    Q_OBJECT
public:
    explicit FilterWorker(const cv::Mat *masterImage = nullptr, cv::Mat *previewImage = nullptr, QMutex *mutex = nullptr, QObject *parent = nullptr);
    ~FilterWorker();

signals:
    void updateDisplayedImage();
    void updateStatus(QString);

public slots:
    void receiveSuppressedSignal(SignalSuppressor *dataContainer);
    void receiveImageAddresses(const cv::Mat *masterImage, cv::Mat *previewImage);

private:
    void performImageFiltering(int *parameter);
    int kernelSize(QSize image, int weightPercent);
    cv::Mat makeLaplacianKernel(int size);
    QByteArray data_m;
    QMutex *mutex_m;
    const cv::Mat *masterImage_m;
    cv::Mat *previewImage_m;
    cv::UMat implicitOclImage_m;
};

#endif // FILTERWORKER_H
