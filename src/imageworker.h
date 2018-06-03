#ifndef IMAGEWORKER_H
#define IMAGEWORKER_H

#include <QObject>
#include <QMutex>
#include <QString>
#include <QDir>
#include <QSize>
#include <opencv2/core/core.hpp>

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
    void doCopyRGBBufferToMasterBuffer();
    void doDisplayMasterBuffer();
    void doAdjustmentsComputation(QVector<float> parameter);
    void doSmoothFilterComputation(QVector<int> parameter);
    void doSharpenFilterComputation(QVector<int> parameter);
    void doEdgeFilterComputation(QVector<int> parameter);
    void doNoiseFilterComputation(QVector<int> parameter);
    void doReconstructFilterComputation(QVector<int> parameter);


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

    //working image buffers (if heap allocated - for sharing between threads)
    std::vector <cv::Mat> splitChannelsTmp;
    cv::Mat *masterRGBImage;
    cv::Mat *srcRGBImage;
    cv::Mat *srcTmpImage;
    cv::Mat *dstRGBImage;
    cv::Mat *dstTmpImage;
    QImage *imageWrapper;


    //working RGB histogram plots
    uint **srcRGBHisto;
    uint **dstRGBHisto; //use openCV to calc histo?

    //mutex for threadsafe operations
    QMutex *mutex;
};

#endif // IMAGEWORKER_H

//get open to work first, then histogram calculation and storage
