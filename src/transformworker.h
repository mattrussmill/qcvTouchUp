#ifndef TRANSFORMWORKER_H
#define TRANSFORMWORKER_H

#include <QObject>
#include <QRect>
#include <opencv2/core.hpp>
class QMutex;
class QString;
class SignalSuppressor;

class TransformWorker : public QObject
{
    Q_OBJECT
public:
    explicit TransformWorker(const cv::Mat *masterImage = nullptr, cv::Mat *previewImage = nullptr, QMutex *mutex = nullptr, QObject *parent = nullptr);
    ~TransformWorker();

signals:
    void updateDisplayedImage();
    void updateStatus(QString);

public slots:
    void receiveSuppressedSignal(SignalSuppressor *dataContainer);
    void receiveImageAddresses(const cv::Mat *masterImage, cv::Mat *previewImage);
    void doCropComputation(QRect);
    void setAutoCropForRotate(bool);
    void doRotateComputation(int);
    void doScaleComputation(QRect);

private:
    QMutex *mutex_m;
    int data_m;
    bool autoCropforRotate_m;
    const cv::Mat *masterImage_m;
    cv::Mat *previewImage_m;
    cv::UMat implicitOclImage_m;

};

#endif // TRANSFORMWORKER_H
