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
    void doAdjustmentsComputation(float *parameterArray);
    void doSmoothFilterComputation(float *parameterArray);
    void doSharpenFilterComputation(float *parameterArray);
    void doEdgeFilterComputation(float *parameterArray);
    void doNoiseFilterComputation(float *parameterArray);
    void doReconstructFilterComputation(float *parameterArray);


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
