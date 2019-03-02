#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "quickmenu.h"
#include "adjustmenu.h"
#include "filtermenu.h"
#include "temperaturemenu.h"
#include "transformmenu.h"
#include "colorslicemenu.h"
#include "bufferwrappersqcv.h"
#include "imagewidget.h"
#include "histogramwindow.h"
#include "imageworker.h"
#include <QWidget>
#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QFileInfo>
#include <QThread>
#include <QDebug>
#include <QDir>
#include <QString>
#include <QImage>
#include <QMutex>
#include <QRect>
#include <QStackedWidget>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgcodecs/imgcodecs.hpp>


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    //main operation setup, members, and mutex
    ui->setupUi(this);
    setWindowTitle("qcvTouchUp");
    userImagePath_m = QDir::homePath();
    ui->imageWidget->setMutex(mutex_m);

    masterRGBImage_m = cv::Mat(0, 0, CV_8UC3);
    previewRGBImage_m = cv::Mat(0, 0, CV_8UC3);

    //setup worker thread event loop for ImageWorker
/*    imageWorker_m = new ImageWorker(mutex_m);
    imageWorker_m->moveToThread(&workerThread);
    connect(&workerThread, SIGNAL(finished()), imageWorker_m, SLOT(deleteLater()));*/ //how to structure this -> look at example again

    //image menus initializations - signals are connected after to not be emitted during initialization
    adjustMenu_m = new AdjustMenu(&mutex_m, this);
    ui->toolMenu->addWidget(adjustMenu_m);
    filterMenu_m = new FilterMenu(this);
    ui->toolMenu->addWidget(filterMenu_m);
    temperatureMenu_m = new TemperatureMenu(this);
    ui->toolMenu->addWidget(temperatureMenu_m);
    transformMenu_m = new TransformMenu(this);
    ui->toolMenu->addWidget(transformMenu_m);
    colorSliceMenu_m = new ColorSliceMenu(this);
    ui->toolMenu->addWidget(colorSliceMenu_m);


    //connect necessary internal mainwindow/ui slots
    connect(ui->actionZoom_In, SIGNAL(triggered()), ui->imageWidget, SLOT(zoomIn()));
    connect(ui->actionZoom_Out, SIGNAL(triggered()), ui->imageWidget, SLOT(zoomOut()));
    connect(ui->actionZoom_Fit, SIGNAL(triggered()), ui->imageWidget, SLOT(zoomFit()));
    connect(ui->actionZoom_Actual, SIGNAL(triggered()), ui->imageWidget, SLOT(zoomActual()));
    connect(ui->actionOpen, SIGNAL(triggered()), this, SLOT(getImagePath()));
    connect(ui->actionHistogram, SIGNAL(triggered()), this, SLOT(loadHistogramTool()));

    //right side tool menu - mainwindow/ui slots
    connect(ui->quickMenu, SIGNAL(menuItemClicked(int)), ui->toolMenu, SLOT(setCurrentIndex(int)));
    connect(ui->toolMenu, SIGNAL(currentChanged(int)), this, SLOT(cancelPreview()));
    connect(ui->pushButtonCancel, SIGNAL(released()), this, SLOT(cancelPreview()));
    connect(ui->pushButtonApply, SIGNAL(released()), this, SLOT(applyPreviewToMaster()));

    //image widget / area - mainwindow/ui slots
    connect(ui->imageWidget, SIGNAL(imageNull()), this, SLOT(imageOpenOperationFailed()));
    connect(ui->imageWidget, SIGNAL(droppedImagePath(QString)), this, SLOT(loadImageIntoMemory(QString)));
    connect(ui->imageWidget, SIGNAL(droppedImageError()), this, SLOT(imageOpenOperationFailed()));

    //connect necessary worker thread - mainwindow/ui slots
//    connect(&workerThread, SIGNAL(started()), this, SLOT(initializeWorkerThreadData()));
//    connect(imageWorker_m, SIGNAL(resultImageSet(const QImage*)), this, SLOT(updateImageInformation(const QImage*)));
//    connect(imageWorker_m, SIGNAL(resultImageSet(const QImage*)), ui->imageWidget, SLOT(setImage(const QImage*)));
//    connect(imageWorker_m, SIGNAL(resultImageUpdate(const QImage*)), ui->imageWidget, SLOT(updateDisplayedImage(const QImage*)));
//    connect(imageWorker_m, SIGNAL(resultHistoUpdate()), this, SLOT(updateHistogram()));
//    connect(imageWorker_m, SIGNAL(updateStatus(QString)), ui->statusBar, SLOT(showMessage(QString)));

    //connect necessary worker thread - adjustmenu / ui slots
    connect(adjustMenu_m, SIGNAL(updateDisplayedImage()), this, SLOT(displayPreview()));
    connect(ui->pushButtonCancel, SIGNAL(released()), adjustMenu_m, SLOT(initializeSliders()));
    connect(ui->pushButtonApply, SIGNAL(released()), adjustMenu_m, SLOT(initializeSliders()));
    connect(this, SIGNAL(distributeImageBufferAddresses(const cv::Mat*, cv::Mat*)), adjustMenu_m, SLOT(receiveImageAddresses(const cv::Mat*, cv::Mat*)));

    //connect necessary worker thread - filtermenu / ui slots
//    connect(filterMenu_m, SIGNAL(performImageBlur(QVector<int>)), imageWorker_m, SLOT(doSmoothFilterComputation(QVector<int>)));
//    connect(filterMenu_m, SIGNAL(performImageSharpen(QVector<int>)), imageWorker_m, SLOT(doSharpenFilterComputation(QVector<int>)));
//    connect(filterMenu_m, SIGNAL(performImageEdgeDetect(QVector<int>)), imageWorker_m, SLOT(doEdgeFilterComputation(QVector<int>)));
//    connect(ui->pushButtonCancel, SIGNAL(released()), filterMenu_m, SLOT(initializeSliders()));
//    connect(ui->pushButtonApply, SIGNAL(released()), filterMenu_m, SLOT(initializeSliders()));

    //connect necessary worker thread - temperaturemenu / ui slots
//    connect(temperatureMenu_m, SIGNAL(performImageAdjustments(int)), imageWorker_m, SLOT(doTemperatureComputation(int)));
//    connect(ui->pushButtonCancel, SIGNAL(released()), temperatureMenu_m, SLOT(initializeMenu()));
//    connect(ui->pushButtonApply, SIGNAL(released()), temperatureMenu_m, SLOT(initializeMenu()));

    //connect necessary worker thread - transformmenu / ui slots
//    connect(ui->imageWidget, SIGNAL(imageRectRegionSelected(QRect)), transformMenu_m, SLOT(setImageROI(QRect)));
//    connect(transformMenu_m, SIGNAL(giveImageROI(QRect)), ui->imageWidget, SLOT(setRectRegionSelected(QRect)));
//    connect(transformMenu_m, SIGNAL(setGetCoordinateMode(uint)), ui->imageWidget, SLOT(setRetrieveCoordinateMode(uint)));
//    connect(transformMenu_m, SIGNAL(cancelRoiSelection()), imageWorker_m, SLOT(doDisplayMasterBuffer()));
//    connect(transformMenu_m, SIGNAL(performImageCrop(QRect)), imageWorker_m, SLOT(doCropComputation(QRect)));
//    connect(transformMenu_m, SIGNAL(setAutoCropOnRotate(bool)), imageWorker_m, SLOT(setAutoCropForRotate(bool)));
//    connect(transformMenu_m, SIGNAL(performImageRotate(int)), imageWorker_m, SLOT(doRotateComputation(int)));
//    connect(transformMenu_m, SIGNAL(performImageScale(QRect)), imageWorker_m, SLOT(doScaleComputation(QRect)));
//    connect(ui->pushButtonCancel, SIGNAL(released()), transformMenu_m, SLOT(initializeMenu()));
}

MainWindow::~MainWindow()
{
    //end worker thread once event loop finishes
    workerThread.quit();
    workerThread.wait();

    //delete heap data not a child of mainwindow
    delete ui;
}

// When an image fails at being opened, clears the image and generates a warning message box.
void MainWindow::imageOpenOperationFailed()
{
    ui->imageWidget->clearImage();
    masterRGBImage_m.release();
    updateImageInformation(nullptr);
    QMessageBox::warning(this, "Error", "Unable to access desired image.");
}

/* When an image address is passed to this slot, the information on the main window is updated according to that image.
 * this is called when an image is SET from the worker thread. TransformMenu is also updated.*/
void MainWindow::updateImageInformation(const QImage *image)
{
    if(image == nullptr)
    {
        setWindowTitle("qcvTouchUp");
        ui->labelSize->setText("Size:");
        ui->labelType->setText("Type:");
        transformMenu_m->setImageResolution(QRect(-1 , -1, -1, -1));
        colorSliceMenu_m->setImageReference(nullptr);
    }
    else
    {
        setWindowTitle("qcvTouchUp - " + QFileInfo(userImagePath_m.absolutePath()).fileName());
        ui->labelSize->setText("Size: " + QString::number(image->width())+"x"+QString::number(image->height()));
        ui->labelType->setText("Type: " + qcv::getMatType(qcv::qImageToCvMat(*image)));
        transformMenu_m->setImageResolution(image->rect());
        colorSliceMenu_m->setImageReference(image);
    }
}

/* When the worker thread emits its started() signal, the GUI event loop sends the worker thread necessary
 * initialization informatoin (such as shared heap data) that is necessary to maintain correct operation */
void MainWindow::initializeWorkerThreadData()
{
    //signals the worker to set its histogram dst data for the displayed image to the buffer managed by the ui.
    imageWorker_m->doSetHistogramDstAddress(const_cast<uint**>(ui->histo->data()));
}

/* Allows the worker thread to override the initialized value of the mainwindow histogram widget and update
 * what is displayed, forcing a repaint of the widget contents*/
void MainWindow::updateHistogram()
{
    ui->histo->setInitialized(true);
    ui->histo->update();
}

/* Creates a dialog box listing supported file types by OpenCV and a file dialog window. If the open dialog
 * box is closed, the function exits. If the dialog box has a path that is not NULL the absolute path is
 * passed to the image loading function to open the image.*/
void MainWindow::getImagePath()
{
    QDir imagePath;
    imagePath.setPath(QFileDialog::getOpenFileName(this, "Select an Image", userImagePath_m.absolutePath(), //use a tmp image path first
                          "All Files (*);;Bitmap (*.bmp *.dib);;JPEG(*.jpeg *.jpg *.jpe);;"
                          "JPEG 2000 (*.jp2);;OpenEXR (*.exr);;PIF (*.pbm *.pgm *.pnm *.ppm *.pxm);;"
                          "PNG (*.png);;Radiance HDR (*.hdr *.pic);;Sun Raster (*.sr *.ras);;"
                          "TIFF (*.tiff *.tif);;WebP (*.webp)"));

    if(imagePath.absolutePath().isNull()) return;
    loadImageIntoMemory(imagePath.absolutePath());
}

/* Takes an image path and attempts to open it. First the image buffer and path are released so that
 * that if the image fails to be loaded into a cv::Mat it can be detected. If the image is loaded
 * it is converted from a BGR (default for OpenCV) to a RGB color format and then wrapped in a QImage
 * wrapper implicitly sharing the data. If successful it returns true, else false.*/
bool MainWindow::loadImageIntoMemory(QString imagePath)
{
    statusBar()->showMessage("Opening...");
    //while waiting for mutex, process main event loop to keep gui responsive
    while(!mutex_m.tryLock())
        QApplication::processEvents(QEventLoop::AllEvents, 100);

    //clear the image buffer and path. Try to open image in BGR format
    masterRGBImage_m = cv::imread(imagePath.toStdString(), cv::IMREAD_COLOR);
    qDebug() << "MainWindow Images:" << &masterRGBImage_m << &previewRGBImage_m;

    //check if operation was successful
    bool returnSuccess = true;
    if(masterRGBImage_m.empty())
    {
        imageOpenOperationFailed();
        returnSuccess = false;
        emit distributeImageBufferAddresses(nullptr, nullptr);
    }
    else
    {
        //if successfully loaded, convert to RGB color space and wrap in QImage
        cv::cvtColor(masterRGBImage_m, masterRGBImage_m, cv::COLOR_BGR2RGB);
        masterRGBImage_m.copyTo(previewRGBImage_m);
        imageWrapper_m = QImage(qcv::cvMatToQImage(masterRGBImage_m));
    }

    mutex_m.unlock();

    // success ops outside of mutex
    if(returnSuccess)
    {
        ui->imageWidget->setImage(&imageWrapper_m); //mutex operation
        userImagePath_m = imagePath;
        emit distributeImageBufferAddresses(&masterRGBImage_m, &previewRGBImage_m);
    }

    statusBar()->showMessage("");
    ui->imageWidget->setFocus();
    return returnSuccess;
}

// Displays a histogram window with x and y axis plot when triggered.
void MainWindow::loadHistogramTool()
{
    statusBar()->showMessage("Histogram...");
    mutex_m.lock();
    QImage *currentImage = const_cast<QImage*>(ui->imageWidget->displayedImage());
    HistogramWindow *histogramWindow = new HistogramWindow(*currentImage, this);
    mutex_m.unlock();
    histogramWindow->exec();
    statusBar()->showMessage("");
}

/* This slot cancels the image operations by wrapping the Mat in a Qimage and setting the imagewidget
 * to display the master image buffer.*/
void MainWindow::cancelPreview()
{
    while(!mutex_m.tryLock())
        QApplication::processEvents(QEventLoop::AllEvents, 100);
    imageWrapper_m = qcv::cvMatToQImage(masterRGBImage_m);
    masterRGBImage_m.copyTo(previewRGBImage_m);
    mutex_m.unlock();
    ui->imageWidget->setImage(&imageWrapper_m);
}

/* This slot applies the previewed operation of the image to the master buffer by performing a deep
 * copy of the preview to the master buffer. Then it wraps the master buffer as a QImage and displays it*/
void MainWindow::applyPreviewToMaster()
{
    while(!mutex_m.tryLock())
        QApplication::processEvents(QEventLoop::AllEvents, 100);
    masterRGBImage_m = previewRGBImage_m.clone();
    imageWrapper_m = qcv::cvMatToQImage(masterRGBImage_m);
    mutex_m.unlock();
    ui->imageWidget->setImage(&imageWrapper_m);
}

// This slot wraps the preview image buffer in a QImage and displays it via the imageWidget
void MainWindow::displayPreview()
{
    while(!mutex_m.tryLock())
        QApplication::processEvents(QEventLoop::AllEvents, 100);
    imageWrapper_m = qcv::cvMatToQImage(previewRGBImage_m);
    mutex_m.unlock();
    ui->imageWidget->setImage(&imageWrapper_m);
}
