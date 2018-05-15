#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "quickmenu.h"
#include "bufferwrappersqcv.h"
#include "imagewidget.h"
#include "histogramwindow.h"
#include <QWidget>
#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QFileInfo>
#include <QThread>
#include <QDebug>


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    //main operation setup, members, and mutex
    ui->setupUi(this);
    setWindowTitle("qcvTouchUp");
    userImagePath = QDir::homePath();
    ui->iw->setMutex(mutex);

    //setup worker thread event loop for ImageWorker
    imageWorker = new ImageWorker(mutex);
    imageWorker->moveToThread(&workerThread);
    connect(&workerThread, SIGNAL(finished), imageWorker, SLOT(deleteLater()));

    //image menus initializations - signals are connected after to not be emitted during initialization
    adjustMenu = new AdjustMenu(this);
    adjustMenu->setVisible(false);
    ui->horizontalLayoutImageTools->addWidget(adjustMenu);
    filterMenu = new FilterMenu(this);
    filterMenu->setVisible(false);
    ui->horizontalLayoutImageTools->addWidget(filterMenu);

    //connect necessary internal mainwindow/ui slots
    connect(ui->actionZoom_In, SIGNAL(triggered()), ui->iw, SLOT(zoomIn()));
    connect(ui->actionZoom_Out, SIGNAL(triggered()), ui->iw, SLOT(zoomOut()));
    connect(ui->actionZoom_Fit, SIGNAL(triggered()), ui->iw, SLOT(zoomFit()));
    connect(ui->actionZoom_Actual, SIGNAL(triggered()), ui->iw, SLOT(zoomActual()));
    connect(ui->quickMenu, SIGNAL(menuItemClicked(int)), this, SLOT(loadSubMenu(int)));
    connect(ui->iw, SIGNAL(imageNull()), this, SLOT(imageOpenOperationFailed()));

    //connect necessary worker thread - mainwindow/ui slots
    connect(&workerThread, SIGNAL(started()), this, SLOT(initializeWorkerThreadData()));
    connect(imageWorker, SIGNAL(resultImageSet(const QImage*)), this, SLOT(updateImageInformation(const QImage*)));
    connect(imageWorker, SIGNAL(resultImageSet(const QImage*)), ui->iw, SLOT(setImage(const QImage*)));
    connect(imageWorker, SIGNAL(resultImageUpdate(const QImage*)), ui->iw, SLOT(updateDisplayedImage(const QImage*)));
    connect(imageWorker, SIGNAL(resultHistoUpdate()), this, SLOT(updateHistogram()));
    connect(imageWorker, SIGNAL(updateStatus(QString)), ui->statusBar, SLOT(showMessage(QString)));

    //connect necessary worker thread - adjustmenu / ui slots
    connect(adjustMenu, SIGNAL(performImageAdjustments(float*)), imageWorker, SLOT(doAdjustmentsComputation(float*)));
    connect(adjustMenu, SIGNAL(cancelAdjustments()), imageWorker, SLOT(doDisplayMasterBuffer()));
    connect(adjustMenu, SIGNAL(applyAdjustments()), imageWorker, SLOT(doCopyRGBBufferToMasterBuffer()));

    //connect necessary worker thread - filtermenu / ui slots
    connect(filterMenu, SIGNAL(performImageBlur(int*)), imageWorker, SLOT(doSmoothFilterComputation(int*)));
    connect(filterMenu, SIGNAL(performImageSharpen(int*)), imageWorker, SLOT(doSharpenFilterComputation(int*)));
    connect(filterMenu, SIGNAL(performImageEdgeDetect(int*)), imageWorker, SLOT(doEdgeFilterComputation(int*)));
    connect(filterMenu, SIGNAL(performImageNoiseRemove(int*)), imageWorker, SLOT(doNoiseFilterComputation(int*)));
    connect(filterMenu, SIGNAL(performImageReconstruct(int*)), imageWorker, SLOT(doNoiseFilterComputation(int*)));
    connect(filterMenu, SIGNAL(cancelAdjustments()), imageWorker, SLOT(doDisplayMasterBuffer()));
    connect(filterMenu, SIGNAL(applyAdjustments()), imageWorker, SLOT(doCopyRGBBufferToMasterBuffer()));

    //start worker thread event loop
    workerThread.start();
}

MainWindow::~MainWindow()
{
    //end worker thread once event loop finishes
    workerThread.quit();
    workerThread.wait();

    //delete heap data not a child of mainwindow
    delete imageWorker;
    delete ui;
}

void MainWindow::loadSubMenu(int menuIndex)
{
        //switch statement for loading menus
        qDebug() << QString::number(menuIndex);

        if(menuIndex)
            ui->histo->setMinimumWidth(275);
        else
            ui->histo->setMinimumWidth(150);

        adjustMenu->setVisible(false);
        filterMenu->setVisible(false);

        switch(menuIndex)
        {
        case 1:
        {
            adjustMenu->setVisible(true);
            break;
        }
        case 2:
        {
            filterMenu->setVisible(true);
            break;
        }
        default:
        {
            //nothing else to do
            break;
        }
        }
}

void MainWindow::imageOpenOperationFailed()
{
    ui->iw->clearImage();
    updateImageInformation(nullptr);
    QMessageBox::warning(this, "Error", "Unable to access desired image. File type not supported.");
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
        setWindowTitle("qcvTouchUp - " + QFileInfo(userImagePath.absolutePath()).fileName());
        ui->labelSize->setText("Size: " + QString::number(image->width())+"x"+QString::number(image->height()));
        ui->labelType->setText("Type: " + qcv::getMatType(qcv::qImageToCvMat(*image)));
    }
}

/* When the worker thread emits its started() signal, the GUI event loop sends the worker thread necessary
 * initialization informatoin (such as shared heap data) that is necessary to maintain correct operation */
void MainWindow::initializeWorkerThreadData()
{
    //signals the worker to set its histogram dst data for the displayed image to the buffer managed by the ui.
    imageWorker->doSetHistogramDstAddress(const_cast<uint**>(ui->histo->data()));
}

/* Allows the worker thread to override the initialized value of the mainwindow histogram widget and update
 * what is displayed, forcing a repaint of the widget contents*/
void MainWindow::updateHistogram()
{
    ui->histo->setInitialized(true);
    ui->histo->update();
}

// MainWindow's "Open..." action creates a dialog box listing supported file types and a file dialog window.
void MainWindow::on_actionOpen_triggered()
{
    statusBar()->showMessage("Opening...");
    userImagePath.setPath(QFileDialog::getOpenFileName(this, "Select an Image", userImagePath.absolutePath(),
                          "All Files (*);;Bitmap (*.bmp *.dib);;JPEG(*.jpeg *.jpg *.jpe);;"
                          "JPEG 2000 (*.jp2);;OpenEXR (*.exr);;PIF (*.pbm *.pgm *.pnm *.ppm *.pxm);;"
                          "PNG (*.png);;Radiance HDR (*.hdr *.pic);;Sun Raster (*.sr *.ras);;"
                          "TIFF (*.tiff *.tif);;WebP (*.webp)"));
    if(userImagePath.absolutePath().isNull())
    {
        return;
        statusBar()->showMessage("");
    }
    ui->iw->clearImage();
    ui->histo->clear();
    imageWorker->doOpenImage(userImagePath.absolutePath());
}




void MainWindow::on_actionAbout_triggered()
{
    QString testDir("U:/miller/Pictures/Dual Monitor/1227736622214.jpg");
    //QString testDir("U:/miller/Pictures/Pictures Taken/100KC743/100_0579.JPG");
    imageWorker->doOpenImage(testDir);


    //GO FIX MENU BACKEND
}

// Displays a histogram window with x and y axis plot when triggered.
void MainWindow::on_actionHistogram_triggered()
{
    statusBar()->showMessage("Histogram...");
    mutex.lock();
    QImage *currentImage = const_cast<QImage*>(ui->iw->displayedImage());
    HistogramWindow *histogramWindow = new HistogramWindow(*currentImage, this);
    mutex.unlock();
    histogramWindow->exec();
    statusBar()->showMessage("");
}
