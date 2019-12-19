/***********************************************************************
* FILENAME :    mainwindow.h
*
* LICENSE:
*       qcvTouchUp provides an image processing toolset for editing
*       photographs, purposed and packaged for use in a desktop application
*       user environment. Copyright (C) 2018,  Matthew R. Miller
*
*       This program is free software: you can redistribute it and/or modify
*       it under the terms of the GNU General Public License as published by
*       the Free Software Foundation (version 3 of the License).
*
*       The framework and libraries used to create this software are licenced
*       under the  GNU Lesser General Public License (LGPL) version 3 and the
*       3-clause BSD License as agreed upon through the use of the Qt toolkit
*       and OpenCV libraries respectively. Copies of the appropriate licenses
*       for qcvTouchup, and its source code, can be found in LICENSE.txt,
*       LICENSE.Qt.txt, and LICENSE.CV.txt. If not, please see
*       <http://www.gnu.org/licenses/> and <https://opencv.org/license.html>
*       for further information on licensing.
*
*       This program is distributed in the hope that it will be useful,
*       but WITHOUT ANY WARRANTY; without even the implied warranty of
*       MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*       GNU General Public License for more details.
*
*       If you wish to contact the developer about this project, please do so
*       through their account at <https://github.com/mattrussmill>
*
* DESCRIPTION :
*       This is the main window of the application. It is used as the parent
*       object for the application. All object placement and communication
*       can be traced back to this object.
*
* AUTHOR :  Matthew R. Miller       START DATE :    March 03/04/2019
*
* CHANGES : N/A - N/A
*
* VERSION       DATE            WHO                     DETAIL
* 0.1           11/03/2019      Matthew R. Miller       Initial Rev
*
************************************************************************/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "nav/quickmenu.h"
#include "editing/menus/adjustmenu.h"
#include "editing/menus/filtermenu.h"
#include "editing/menus/temperaturemenu.h"
#include "editing/menus/transformmenu.h"
//#include "colorslicemenu.h"
#include "bufferwrappersqcv.h"
#include "imagewidget.h"
#include "dialog/imagesavedialog.h"
#include "imagelabel.h"
#include <QWidget>
#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QFileInfo>
#include <QDebug>
#include <QDir>
#include <QString>
#include <QImage>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/core/ocl.hpp>
#include <QHBoxLayout>

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

    //image menus initializations - signals are connected after to not be emitted during initialization
    adjustMenu_m = new AdjustMenu(&mutex_m, this);
    ui->toolMenu->addWidget(adjustMenu_m);
    filterMenu_m = new FilterMenu(&mutex_m, this);
    ui->toolMenu->addWidget(filterMenu_m);
    temperatureMenu_m = new TemperatureMenu(&mutex_m, this);
    ui->toolMenu->addWidget(temperatureMenu_m);
    transformMenu_m = new TransformMenu(&mutex_m, this);
    ui->toolMenu->addWidget(transformMenu_m);
    //colorSliceMenu_m = new ColorSliceMenu(this);
    //ui->toolMenu->addWidget(colorSliceMenu_m);


    //connect necessary internal mainwindow/ui slots
    connect(ui->actionZoom_In, &QAction::triggered, [=](){ui->imageWidget->zoomIn(ui->imageWidget->rect().center());}); //lambda
    connect(ui->actionZoom_Out, &QAction::triggered, [=](){ui->imageWidget->zoomOut(ui->imageWidget->rect().center());}); //lambda
    connect(ui->actionZoom_Fit, SIGNAL(triggered()), ui->imageWidget, SLOT(zoomFit()));
    connect(ui->actionZoom_Actual, SIGNAL(triggered()), ui->imageWidget, SLOT(zoomActual()));
    connect(ui->actionOpen, SIGNAL(triggered()), this, SLOT(getImagePath()));
    connect(ui->actionSave_As, SIGNAL(triggered()), this, SLOT(saveImageAs()));
    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(generateAboutDialog()));

    //right side tool menu - mainwindow/ui slots
    connect(ui->quickMenu, SIGNAL(menuItemClicked(int)), ui->toolMenu, SLOT(setCurrentIndex(int)));
    connect(ui->toolMenu, SIGNAL(currentChanged(int)), this, SLOT(cancelPreview()));
    connect(ui->pushButtonCancel, SIGNAL(released()), this, SLOT(cancelPreview()));
    connect(ui->pushButtonApply, SIGNAL(released()), this, SLOT(applyPreviewToMaster()));
    connect(ui->actionAdjust, &QAction::triggered, [=](){ui->toolMenu->setCurrentIndex(0);}); //lambda
    connect(ui->actionFilter, &QAction::triggered, [=](){ui->toolMenu->setCurrentIndex(1);}); //lambda
    connect(ui->actionTemperature, &QAction::triggered, [=](){ui->toolMenu->setCurrentIndex(2);}); //lambda
    connect(ui->actionTransform, &QAction::triggered, [=](){ui->toolMenu->setCurrentIndex(3);}); //lambda

    //image widget / area - mainwindow/ui slots
    connect(ui->imageWidget, SIGNAL(imageNull()), this, SLOT(imageOpenOperationFailed()));
    connect(ui->imageWidget, SIGNAL(droppedImagePath(QString)), this, SLOT(loadImageIntoMemory(QString)));
    connect(ui->imageWidget, SIGNAL(droppedImageError()), this, SLOT(imageOpenOperationFailed()));

    //connect necessary adjustmenu / ui slots
    connect(ui->pushButtonCancel, SIGNAL(released()), adjustMenu_m, SLOT(initializeSliders()));
    connect(ui->pushButtonApply, SIGNAL(released()), adjustMenu_m, SLOT(initializeSliders()));
    connect(adjustMenu_m, SIGNAL(updateDisplayedImage()), this, SLOT(displayPreview()));
    connect(adjustMenu_m, SIGNAL(updateStatus(QString)), ui->statusBar, SLOT(showMessage(QString)));
    connect(this, SIGNAL(setDefaultTracking(bool)), adjustMenu_m, SLOT(setMenuTracking(bool)));
    connect(this, SIGNAL(distributeImageBufferAddresses(const cv::Mat*, cv::Mat*)), adjustMenu_m, SLOT(initializeSliders()));
    connect(this, SIGNAL(distributeImageBufferAddresses(const cv::Mat*, cv::Mat*)), adjustMenu_m, SLOT(receiveImageAddresses(const cv::Mat*, cv::Mat*)));

    //connect necessary filtermenu / ui slots
    connect(ui->pushButtonCancel, SIGNAL(released()), filterMenu_m, SLOT(initializeSliders()));
    connect(ui->pushButtonApply, SIGNAL(released()), filterMenu_m, SLOT(initializeSliders()));
    connect(filterMenu_m, SIGNAL(updateDisplayedImage()), this, SLOT(displayPreview()));
    connect(filterMenu_m, SIGNAL(updateStatus(QString)), ui->statusBar, SLOT(showMessage(QString)));
    connect(this, SIGNAL(setDefaultTracking(bool)), filterMenu_m, SLOT(setMenuTracking(bool)));
    connect(this, SIGNAL(distributeImageBufferAddresses(const cv::Mat*, cv::Mat*)), filterMenu_m, SLOT(initializeSliders()));
    connect(this, SIGNAL(distributeImageBufferAddresses(const cv::Mat*, cv::Mat*)), filterMenu_m, SLOT(receiveImageAddresses(const cv::Mat*, cv::Mat*)));

    //connect necessary temperaturemenu / ui slots
    connect(ui->pushButtonCancel, SIGNAL(released()), temperatureMenu_m, SLOT(initializeSliders()));
    connect(ui->pushButtonApply, SIGNAL(released()), temperatureMenu_m, SLOT(initializeSliders()));
    connect(temperatureMenu_m, SIGNAL(updateDisplayedImage()), this, SLOT(displayPreview()));
    connect(temperatureMenu_m, SIGNAL(updateStatus(QString)), ui->statusBar, SLOT(showMessage(QString)));
    connect(this, SIGNAL(setDefaultTracking(bool)), temperatureMenu_m, SLOT(setMenuTracking(bool)));
    connect(this, SIGNAL(distributeImageBufferAddresses(const cv::Mat*, cv::Mat*)), temperatureMenu_m, SLOT(initializeSliders()));
    connect(this, SIGNAL(distributeImageBufferAddresses(const cv::Mat*, cv::Mat*)), temperatureMenu_m, SLOT(receiveImageAddresses(const cv::Mat*, cv::Mat*)));

    //connect necessary transformmenu / ui slots
    connect(ui->pushButtonCancel, SIGNAL(released()), transformMenu_m, SLOT(initializeSliders()));
    connect(ui->pushButtonApply, SIGNAL(released()), transformMenu_m, SLOT(initializeSliders()));
    connect(transformMenu_m, SIGNAL(updateDisplayedImage()), this, SLOT(displayPreview()));
    connect(transformMenu_m, SIGNAL(updateStatus(QString)), ui->statusBar, SLOT(showMessage(QString)));
    connect(transformMenu_m, SIGNAL(displayMaster()), this, SLOT(cancelPreview()));
    connect(this, SIGNAL(setDefaultTracking(bool)), transformMenu_m, SLOT(setMenuTracking(bool)));
    connect(this, SIGNAL(distributeImageBufferAddresses(const cv::Mat*, cv::Mat*)), transformMenu_m, SLOT(initializeSliders()));
    connect(this, SIGNAL(distributeImageBufferAddresses(const cv::Mat*, cv::Mat*)), transformMenu_m, SLOT(receiveImageAddresses(const cv::Mat*, cv::Mat*)));
    connect(ui->imageWidget->imageLabel_m, SIGNAL(imageRectRegionSelected(QRect)), transformMenu_m, SLOT(setImageROI(QRect)));
    connect(transformMenu_m, SIGNAL(giveImageROI(QRect)), ui->imageWidget->imageLabel_m, SLOT(setRectRegionSelected(QRect)));
    connect(transformMenu_m, SIGNAL(setGetCoordinateMode(uint)), ui->imageWidget->imageLabel_m, SLOT(setRetrieveCoordinateMode(uint)));

    //Check if OpenCL acceleration is available. If it is available, enable menu item tracking for all child menues, else disable
    cv::ocl::Context ctx = cv::ocl::Context::getDefault();
    if (!ctx.ptr())
    {
        statusBar()->showMessage("OpenCL is not available", 3000);
        ui->actionTracking->setEnabled(false);
        emit setDefaultTracking(false);
    }
    else
    {
        connect(ui->actionTracking, SIGNAL(toggled(bool)), this, SIGNAL(setDefaultTracking(bool)));
        ui->actionTracking->setChecked(true);
    }
}

//delete heap data not a child of mainwindow
MainWindow::~MainWindow()
{
    delete ui;
}

// When an image fails at being opened, clears the image and generates a warning message box.
void MainWindow::imageOpenOperationFailed()
{
    ui->imageWidget->clearImage();
    masterRGBImage_m.release();
    previewRGBImage_m.release();
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
        //colorSliceMenu_m->setImageReference(nullptr);
    }
    else
    {
        setWindowTitle("qcvTouchUp - " + QFileInfo(userImagePath_m.absolutePath()).fileName());
        ui->labelSize->setText("Size: " + QString::number(image->width())+"x"+QString::number(image->height()));
        ui->labelType->setText("Type: " + qcv::getMatType(qcv::qImageToCvMat(*image)));
        transformMenu_m->setImageResolution(image->rect());
        //colorSliceMenu_m->setImageReference(image);
    }
}

/* Creates a dialog box listing supported file types by OpenCV and a file dialog window. If the open dialog
 * box is closed, the function exits. If a file is selected, the absolute path is passed to the image loading
 * function to open the image.*/
void MainWindow::getImagePath()
{
    QDir imagePath;
    QFileDialog openFileDialog(this, "Select an Image", userImagePath_m.absolutePath(),
                               "All Files (*);;Bitmap (*.bmp *.dib);;JPEG(*.jpeg *.jpg *.jpe);;"
                               "JPEG 2000 (*.jp2);;OpenEXR (*.exr);;PIF (*.pbm *.pgm *.pnm *.ppm *.pxm);;"
                               "PNG (*.png);;Radiance HDR (*.hdr *.pic);;Sun Raster (*.sr *.ras);;"
                               "TIFF (*.tiff *.tif);;WebP (*.webp)");
    connect(&openFileDialog, SIGNAL(accepted()), this, SLOT(loadImageIntoMemory()));
    openFileDialog.setOption(QFileDialog::DontUseNativeDialog);
    openFileDialog.setAcceptMode(QFileDialog::AcceptOpen);

    if(openFileDialog.exec() == QDialog::Accepted)
    {
        qDebug() << "open image path: " << openFileDialog.selectedFiles().at(0);
        loadImageIntoMemory(openFileDialog.selectedFiles().at(0));
    }
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
    userImagePath_m = QDir::homePath();
    previewRGBImage_m.release();
    masterRGBImage_m.release();
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
        updateImageInformation(&imageWrapper_m);
    }

    statusBar()->showMessage("");
    ui->imageWidget->setFocus();
    return returnSuccess;
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
    masterRGBImage_m.release();
    previewRGBImage_m.copyTo(masterRGBImage_m);
    imageWrapper_m = qcv::cvMatToQImage(masterRGBImage_m);
    mutex_m.unlock();
    updateImageInformation(&imageWrapper_m);
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

/* saveImageAs first acquires the mutex, then converts the image from the RGB format used to display
 * the imagein Qt to BGR which is the OpenCV format. Then the method launches the save dialog to
 * perform the save operation */
void MainWindow::saveImageAs()
{
    qDebug() << userImagePath_m.absolutePath();

    while(!mutex_m.tryLock())
        QApplication::processEvents(QEventLoop::AllEvents, 100);

    statusBar()->showMessage("Saving...");

    //first change back to OpenCV color space, save in preview, and display master (user must hit apply before saving)
    imageWrapper_m = qcv::cvMatToQImage(masterRGBImage_m);
    updateImageInformation(&imageWrapper_m);
    cv::cvtColor(masterRGBImage_m, previewRGBImage_m, cv::COLOR_RGB2BGR);

    //launch the save dialog with the correct BGR image format in the preview
    ImageSaveDialog saveDialog(previewRGBImage_m, this, "Save As", userImagePath_m.absolutePath());
    saveDialog.exec();
    cv::cvtColor(previewRGBImage_m, previewRGBImage_m, cv::COLOR_BGR2RGB);
    mutex_m.unlock();
    statusBar()->showMessage("");
}

//presents general and legal information about the application
void MainWindow::generateAboutDialog()
{
    QMessageBox::about(this, "About", "qcvTouchUp is released under the GPLv3 license made possible by the "
                                      "<a href='https://www.qt.io/'>Qt Framework</a> via the following "
                                      "<a href='https://doc.qt.io/qt-5/licensing.html'>LGPLv3 license</a> and "
                                      "<a href='https://opencv.org/'>OpenCV Library</a> via the following "
                                      "<a href='https://opencv.org/license/'>3-Clause BSD license</a>. "
                                      "License files have been provided with the installation package as well "
                                      "as with the <a href='https://github.com/mattrussmill/qcvTouchUp'>source code</a>. ");
}
