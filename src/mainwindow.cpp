#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "quickmenu.h"
#include "adjustmenu.h"
#include "filtermenu.h"
#include "temperaturemenu.h"
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


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    //main operation setup, members, and mutex
    ui->setupUi(this);
    setWindowTitle("qcvTouchUp");
    userImagePath_m = QDir::homePath();
    ui->iw->setMutex(mutex);

    //setup worker thread event loop for ImageWorker
    imageWorker_m = new ImageWorker(mutex);
    imageWorker_m->moveToThread(&workerThread);
    connect(&workerThread, SIGNAL(finished), imageWorker_m, SLOT(deleteLater()));

    //image menus initializations - signals are connected after to not be emitted during initialization
    adjustMenu_m = new AdjustMenu(this);
    adjustMenu_m->setVisible(false);
    ui->horizontalLayoutImageTools->addWidget(adjustMenu_m);
    filterMenu_m = new FilterMenu(this);
    filterMenu_m->setVisible(false);
    ui->horizontalLayoutImageTools->addWidget(filterMenu_m);
    temperatureMenu_m = new TemperatureMenu(this);
    temperatureMenu_m->setVisible(false);
    ui->horizontalLayoutImageTools->addWidget(temperatureMenu_m);

    //connect necessary internal mainwindow/ui slots
    connect(ui->actionZoom_In, SIGNAL(triggered()), ui->iw, SLOT(zoomIn()));
    connect(ui->actionZoom_Out, SIGNAL(triggered()), ui->iw, SLOT(zoomOut()));
    connect(ui->actionZoom_Fit, SIGNAL(triggered()), ui->iw, SLOT(zoomFit()));
    connect(ui->actionZoom_Actual, SIGNAL(triggered()), ui->iw, SLOT(zoomActual()));
    connect(ui->actionOpen, SIGNAL(triggered()), this, SLOT(openImage()));
    connect(ui->actionHistogram, SIGNAL(triggered()), this, SLOT(loadHistogramTool()));
    connect(ui->quickMenu, SIGNAL(menuItemClicked(int)), this, SLOT(loadSubMenu(int)));
    connect(ui->iw, SIGNAL(imageNull()), this, SLOT(imageOpenOperationFailed()));
    connect(ui->iw, SIGNAL(droppedImagePath(QString)), this, SLOT(openImage(QString)));
    connect(ui->iw, SIGNAL(droppedImageError()), this, SLOT(imageOpenOperationFailed()));

    //connect necessary worker thread - mainwindow/ui slots
    connect(&workerThread, SIGNAL(started()), this, SLOT(initializeWorkerThreadData()));
    connect(imageWorker_m, SIGNAL(resultImageSet(const QImage*)), this, SLOT(updateImageInformation(const QImage*)));
    connect(imageWorker_m, SIGNAL(resultImageSet(const QImage*)), ui->iw, SLOT(setImage(const QImage*)));
    connect(imageWorker_m, SIGNAL(resultImageUpdate(const QImage*)), ui->iw, SLOT(updateDisplayedImage(const QImage*)));
    connect(imageWorker_m, SIGNAL(resultHistoUpdate()), this, SLOT(updateHistogram()));
    connect(imageWorker_m, SIGNAL(updateStatus(QString)), ui->statusBar, SLOT(showMessage(QString)));

    //connect necessary worker thread - adjustmenu / ui slots
    connect(adjustMenu_m, SIGNAL(performImageAdjustments(QVector<float>)), imageWorker_m, SLOT(doAdjustmentsComputation(QVector<float>)));
    connect(adjustMenu_m, SIGNAL(cancelAdjustments()), imageWorker_m, SLOT(doDisplayMasterBuffer()));
    connect(adjustMenu_m, SIGNAL(applyAdjustments()), imageWorker_m, SLOT(doCopyRGBBufferToMasterBuffer()));

    //connect necessary worker thread - filtermenu / ui slots
    connect(filterMenu_m, SIGNAL(performImageBlur(QVector<int>)), imageWorker_m, SLOT(doSmoothFilterComputation(QVector<int>)));
    connect(filterMenu_m, SIGNAL(performImageSharpen(QVector<int>)), imageWorker_m, SLOT(doSharpenFilterComputation(QVector<int>)));
    connect(filterMenu_m, SIGNAL(performImageEdgeDetect(QVector<int>)), imageWorker_m, SLOT(doEdgeFilterComputation(QVector<int>)));
    connect(filterMenu_m, SIGNAL(cancelAdjustments()), imageWorker_m, SLOT(doDisplayMasterBuffer()));
    connect(filterMenu_m, SIGNAL(applyAdjustments()), imageWorker_m, SLOT(doCopyRGBBufferToMasterBuffer()));

    //connect necessary worker thread - temperaturemenu / ui slots
    connect(temperatureMenu_m, SIGNAL(performImageAdjustments(int)), imageWorker_m, SLOT(doTemperatureComputation(int)));
    connect(temperatureMenu_m, SIGNAL(cancelAdjustments()), imageWorker_m, SLOT(doDisplayMasterBuffer()));
    connect(temperatureMenu_m, SIGNAL(applyAdjustments()), imageWorker_m, SLOT(doCopyRGBBufferToMasterBuffer()));

    //start worker thread event loop
    workerThread.start();
}

MainWindow::~MainWindow()
{
    //end worker thread once event loop finishes
    workerThread.quit();
    workerThread.wait();

    //delete heap data not a child of mainwindow
    delete imageWorker_m;
    delete ui;
}

void MainWindow::loadSubMenu(int menuIndex)
{
        //switch statement for loading menus
        qDebug() << "Menu Number:" << QString::number(menuIndex);

        if(menuIndex)
            ui->histo->setMinimumWidth(275);
        else
            ui->histo->setMinimumWidth(150);

        adjustMenu_m->setVisible(false);
        filterMenu_m->setVisible(false);
        temperatureMenu_m->setVisible(false);

        switch(menuIndex)
        {
        case 1:
        {
            adjustMenu_m->setVisible(true);
            break;
        }
        case 2:
        {
            filterMenu_m->setVisible(true);
            break;
        }
        case 3:
        {
            temperatureMenu_m->setVisible(true);
        }
        default:
        {
            //nothing else to do
            break;
        }
        }
}

// When an image fails at being opened, clears the image and generates a warning message box.
void MainWindow::imageOpenOperationFailed()
{
    ui->iw->clearImage();
    updateImageInformation(nullptr);
    QMessageBox::warning(this, "Error", "Unable to access desired image.");
}

/* When an image address is passed to this slot, the information on the main window is updated according to that image.
 * this is called when an image is SET from the worker thread.*/
void MainWindow::updateImageInformation(const QImage *image)
{
    if(image == nullptr)
    {
        setWindowTitle("qcvTouchUp");
        ui->labelSize->setText("Size:");
        ui->labelType->setText("Type:");
    }
    else
    {
        setWindowTitle("qcvTouchUp - " + QFileInfo(userImagePath_m.absolutePath()).fileName());
        ui->labelSize->setText("Size: " + QString::number(image->width())+"x"+QString::number(image->height()));
        ui->labelType->setText("Type: " + qcv::getMatType(qcv::qImageToCvMat(*image)));
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
 * box is closed, the status bar is cleared and function exits. If the dialog box has a path that is not NULL
 * the absolute path is passed to the worker thread to open the image. */
void MainWindow::openImage()
{
    statusBar()->showMessage("Opening...");
    userImagePath_m.setPath(QFileDialog::getOpenFileName(this, "Select an Image", userImagePath_m.absolutePath(),
                          "All Files (*);;Bitmap (*.bmp *.dib);;JPEG(*.jpeg *.jpg *.jpe);;"
                          "JPEG 2000 (*.jp2);;OpenEXR (*.exr);;PIF (*.pbm *.pgm *.pnm *.ppm *.pxm);;"
                          "PNG (*.png);;Radiance HDR (*.hdr *.pic);;Sun Raster (*.sr *.ras);;"
                          "TIFF (*.tiff *.tif);;WebP (*.webp)"));
    if(userImagePath_m.absolutePath().isNull())
    {
        return;
        statusBar()->showMessage("");
    }
    ui->iw->clearImage();
    ui->histo->clear();
    imageWorker_m->doOpenImage(userImagePath_m.absolutePath());
}

/* An overload of openImage without the use of a dialog box. Takes the image path directly as a parameter
 * and preps the image widget for a new image. Then passes the image path to the worker thread to open the
 * image. */
void MainWindow::openImage(QString imagePath)
{
    if(imagePath.isEmpty())
    {
        statusBar()->showMessage("");
        imageOpenOperationFailed();
        return;
    }
    ui->iw->clearImage();
    ui->histo->clear();
    userImagePath_m.setPath(imagePath);
    imageWorker_m->doOpenImage(imagePath);
}

// Displays a histogram window with x and y axis plot when triggered.
void MainWindow::loadHistogramTool()
{
    statusBar()->showMessage("Histogram...");
    mutex.lock();
    QImage *currentImage = const_cast<QImage*>(ui->iw->displayedImage());
    HistogramWindow *histogramWindow = new HistogramWindow(*currentImage, this);
    mutex.unlock();
    histogramWindow->exec();
    statusBar()->showMessage("");
}
