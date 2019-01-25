#ifndef IMAGEWORKER_H
#define IMAGEWORKER_H

#include <opencv2/core/core.hpp>
#include <QObject>
#include <QRect>
class QString;
class QDir;
class QSize;
class QMutex;
//class SignalSuppressor;


class ImageWorker : public QObject
{
    Q_OBJECT

public:
    explicit ImageWorker(QMutex &m);
    ~ImageWorker();

public slots:
    void doOpenImage(const QString imagePath);
    void doClearImageBuffer();
    void doSetHistogramDstAddress(uint **histo);
    void doCopyWorkerBufferToMasterBuffer();//change name please? Or make internal call from APPLY SLOT
    void doDisplayMasterBuffer();
    void doAdjustmentsComputation(QVector<float> parameter);
    void doSmoothFilterComputation(QVector<int> parameter);
    void doSharpenFilterComputation(QVector<int> parameter);
    void doEdgeFilterComputation(QVector<int> parameter);
    void doTemperatureComputation(int parameter);
    void doCropComputation(QRect roi);
    void doRotateComputation(int degree);
    void setAutoCropForRotate(bool value);
    void doScaleComputation(QRect newSize);

signals:
    void resultImageSet(const QImage*);
    void resultImageUpdate(const QImage*);
    void resultHistoUpdate();
    void updateStatus(const QString);



private:
    //non mutexed member functions for use within a mutex lock
    inline void clearImageBuffers();
    inline void deriveWorkingBuffersFromMaster();
    void sortHistogramBuckets(float (ImageWorker::*newIntensityFunction)(int, void*), uint** source,
                              uint** destination, void *fnPtrParameter = nullptr, int numberOfChannels = 3);
    float histogramContrastSorter(int pixel, void *alpha);
    float histogramBrightnessSorter(int pixel, void *beta);
    int kernelSize(QSize image, int weightPercent);
    cv::Mat makeLaplacianKernel(int size);
    bool preImageOperationMutex();
    void postImageOperationMutex();
    cv::Rect innerRectOfRotatedRect(cv::Point2f corners[]);

    //working image buffers (if heap allocated - for sharing between threads)
    std::vector <cv::Mat> splitChannelsTmp_m;
    cv::Mat *masterRGBImage_m;
    cv::Mat *srcRGBImage_m;
    cv::Mat *tmpImage_m;
    cv::Mat *dstRGBImage_m;
    QImage *imageWrapper_m;
    bool autoCropforRotate_m;
    cv::Rect tmpRect;

    //working RGB histogram plots
    uint **srcRGBHisto_m;
    uint **dstRGBHisto_m; //use openCV to calc histo?

    //mutex for threadsafe operations
    QMutex *mutex_m;
};

#endif // IMAGEWORKER_H

//get open to work first, then histogram calculation and storage
