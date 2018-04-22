#ifndef WORKEREVENTLOOP_H
#define WORKEREVENTLOOP_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <opencv2/core/core.hpp>
#include "bufferwrappersqcv.h"

class WorkerEventLoop : public QObject
{
    Q_OBJECT
    QThread workerThread;

public:
    explicit WorkerEventLoop(QMutex &m);
    ~WorkerEventLoop();

public slots:
    void handleImageSetResult(const QImage *image);
    void handleImageUpdateResult(const QImage *image);

signals:
    void workerReady();
    void operateSetWorkerHistoBufferDest(uint**);
    void setDisplayedImage(const QImage*);
    void operateOpenImage(const QString);
    void finishedImageOpenOperation();
    void updateDisplayedImage(const QImage*);
    void updateStatusBar(const QString);
    void operateClearImageBuffer();
    void updateImageInformation(const QImage*);
    void updateDisplayedHistogram();
    void operateApplyRGBChangesToMasterBuffer();
    void operateDisplayMasterBuffer();
    void operateAdjustments(float*);
    void operateSmoothImage(int*);
    void operateSharpenImage(int*);
    void operateEdgeDetectImage(int*);
    void operateNoiseRemoveImage(int*);
    void operateReconstructImage(int*);



};

#endif // WORKEREVENTLOOP_H
