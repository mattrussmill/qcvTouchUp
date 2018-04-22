#include "workereventloop.h"
#include "imageworker.h"
#include <QDebug>

//put comment here about thread afinity and event loop with need for stuff laskdgkjhasdkljhfkalsjhdf
WorkerEventLoop::WorkerEventLoop(QMutex &m)
{
    ImageWorker *worker = new ImageWorker(m);
    worker->moveToThread(&workerThread);

    //deleteLater postpones the deletion of worker until the event loop ends
    connect(&workerThread, SIGNAL(finished), worker, SLOT(deleteLater));

    //connect the signal for passing an image/information to the mainEventLoop to be displayed in the GUI
    connect(&workerThread, SIGNAL(started()), this, SIGNAL(workerReady()));
    connect(this, SIGNAL(operateSetWorkerHistoBufferDest(uint**)), worker, SLOT(doSetHistogramDstAddress(uint**)));
    connect(worker, SIGNAL(resultImageSet(const QImage*)), this, SLOT(handleImageSetResult(const QImage*)));
    connect(worker, SIGNAL(resultImageUpdate(const QImage*)), this, SLOT(handleImageUpdateResult(const QImage*)));
    connect(worker, SIGNAL(resultHistoUpdate()), this, SIGNAL(updateDisplayedHistogram()));
    connect(worker, SIGNAL(updateStatus(QString)), this, SIGNAL(updateStatusBar(QString)));
    connect(this, SIGNAL(operateDisplayMasterBuffer()), worker, SLOT(doDisplayMasterBuffer()));

    //connect the signals and slots related to image I/O
    connect(this, SIGNAL(operateOpenImage(const QString)), worker, SLOT(doOpenImage(const QString)));

    //connect signal to manipulate and move the image buffers
    connect(this, SIGNAL(operateClearImageBuffer()), worker, SLOT(doClearImageBuffer()));
    connect(this, SIGNAL(operateApplyRGBChangesToMasterBuffer()), worker, SLOT(doCopyRGBBufferToMasterBuffer()));

    //connect adjustmenu signals
    connect(this, SIGNAL(operateAdjustments(float*)), worker, SLOT(doAdjustmentsComputation(float*)));

    //connect filtermenu signals
    connect(this, SIGNAL(operateSmoothImage(int*)), worker, SLOT(doSmoothFilterComputation(int*)));
    connect(this, SIGNAL(operateSharpenImage(int*)), worker, SLOT(doSharpenFilterComputation(int*)));
    connect(this, SIGNAL(operateEdgeDetectImage(int*)), worker, SLOT(doEdgeFilterComputation(int*)));
    connect(this, SIGNAL(operateNoiseRemoveImage(int*)), worker, SLOT(doNoiseFilterComputation(int*)));
    connect(this, SIGNAL(operateReconstructImage(int*)), worker, SLOT(doReconstructFilterComputation(int*)));

    workerThread.start();
}

WorkerEventLoop::~WorkerEventLoop()
{
    workerThread.quit();
    workerThread.wait();
}

void WorkerEventLoop::handleImageSetResult(const QImage *image) {emit setDisplayedImage(image);} // should emit a second signal to update info

void WorkerEventLoop::handleImageUpdateResult(const QImage *image) {emit updateDisplayedImage(image);}
