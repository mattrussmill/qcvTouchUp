/***********************************************************************
* FILENAME :    filtermenu.cpp
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
*       This menu is used for filtering the image through 2D kernels by
*       performing high and low pass filtering. This filtering includes
*       sharpening, smoothing/blurring, and edge detection. It collects
*       all necessary slider values in a QVector and copies the object
*       through the signal/signal mechanism to provide the necessary
*       parameters for image processing.
*
*
* NOTES :
*       This class is directly tied to filtermenu.ui and has functionality
*       tied to the ImageWorker class which uses the values selected here
*       as a basis to control calculations on the image.
*
*
* AUTHOR :  Matthew R. Miller       START DATE :    April 04/16/2018
*
* CHANGES : N/A - N/A
*
* VERSION       DATE            WHO                     DETAIL
* 0.1           06/23/2018      Matthew R. Miller       Initial Rev
*
************************************************************************/

#include "mousewheeleatereventfilter.h"
#include "focusindetectoreventfilter.h"
#include "filtermenu.h"
#include "ui_filtermenu.h"
#include "filterworker.h"
#include <QScrollArea>
#include <QVector>
#include <QPixmap>
#include <QButtonGroup>
#include <QByteArray>
#include <QDebug>

//Constructor initializes all members, installs event filters, and connects necessary signals / slots.
FilterMenu::FilterMenu(QMutex *mutex, QWidget *parent) :
    QScrollArea(parent),
    ui(new Ui::FilterMenu)
{
    ui->setupUi(this);
    masterImage_m = nullptr;
    previewImage_m = nullptr;
    workerMutex_m = mutex;
    filterWorker_m = nullptr;

    MouseWheelEaterEventFilter *wheelFilter = new MouseWheelEaterEventFilter(this);
    FocusInDetectorEventFilter *smoothFocusFilter = new FocusInDetectorEventFilter(this);
    FocusInDetectorEventFilter *sharpenFocusFilter = new FocusInDetectorEventFilter(this);
    FocusInDetectorEventFilter *edgeFocusFilter = new FocusInDetectorEventFilter(this);

    //fix radio buttons to work in separate group boxes (for asthetics)
    buttonGroup_m = new QButtonGroup(this);
    buttonGroup_m->addButton(ui->radioButton_SmoothEnable);
    buttonGroup_m->addButton(ui->radioButton_SharpenEnable);
    buttonGroup_m->addButton(ui->radioButton_EdgeEnable);

    initializeSliders();

    //setup smooth menu options
    ui->comboBox_Smooth->addItem("Average");    //comboBox index 0 (default)
    ui->comboBox_Smooth->addItem("Gaussian");   //comboBox index 1
    ui->comboBox_Smooth->addItem("Median");     //comboBox index 2
    ui->comboBox_Smooth->installEventFilter(wheelFilter);
    ui->horizontalSlider_SmoothWeight->installEventFilter(wheelFilter);
    ui->horizontalSlider_SmoothWeight->installEventFilter(smoothFocusFilter);
    connect(ui->radioButton_SmoothEnable, SIGNAL(released()), this, SLOT(collectBlurParameters()));
    connect(ui->comboBox_Smooth, SIGNAL(currentIndexChanged(int)), this, SLOT(collectBlurParameters()));
    connect(smoothFocusFilter, SIGNAL(focusDetected(bool)), ui->radioButton_SmoothEnable, SLOT(setChecked(bool)));
    connect(smoothFocusFilter, SIGNAL(focusDetected(bool)), this, SLOT(changeSampleImage(bool)));
    connect(ui->horizontalSlider_SmoothWeight, SIGNAL(valueChanged(int)), this, SLOT(collectBlurParameters()));
    connect(ui->radioButton_SmoothEnable, SIGNAL(toggled(bool)), this, SLOT(changeSampleImage(bool)));

    //setup sharpen menu options
    ui->comboBox_Sharpen->addItem("Unsharpen"); //comboBox index 0 (default)
    ui->comboBox_Sharpen->addItem("Laplacian"); //comboBox index 1
    ui->comboBox_Sharpen->installEventFilter(wheelFilter);
    ui->horizontalSlider_SharpenWeight->installEventFilter(wheelFilter);
    ui->horizontalSlider_SharpenWeight->installEventFilter(sharpenFocusFilter);
    connect(ui->radioButton_SharpenEnable, SIGNAL(released()), this, SLOT(collectSharpenParameters()));
    connect(ui->comboBox_Sharpen, SIGNAL(currentIndexChanged(int)), this, SLOT(adjustSharpenSliderRange(int)));
    connect(ui->comboBox_Sharpen, SIGNAL(currentIndexChanged(int)), this, SLOT(collectSharpenParameters()));
    connect(sharpenFocusFilter, SIGNAL(focusDetected(bool)), ui->radioButton_SharpenEnable, SLOT(setChecked(bool)));
    connect(sharpenFocusFilter, SIGNAL(focusDetected(bool)), this, SLOT(changeSampleImage(bool)));
    connect(ui->horizontalSlider_SharpenWeight, SIGNAL(valueChanged(int)), this, SLOT(collectSharpenParameters()));
    connect(ui->radioButton_SharpenEnable, SIGNAL(toggled(bool)), this, SLOT(changeSampleImage(bool)));

    //setup edge detect menu options
    ui->comboBox_Edge->addItem("Canny");        //comboBox index 0 (default)
    ui->comboBox_Edge->addItem("Laplacian");    //comboBox index 1 - must match above
    ui->comboBox_Edge->addItem("Sobel");        //comboBox index 2
    ui->comboBox_Edge->installEventFilter(wheelFilter);
    ui->horizontalSlider_EdgeWeight->installEventFilter(wheelFilter);
    ui->horizontalSlider_EdgeWeight->installEventFilter(edgeFocusFilter);
    connect(ui->radioButton_EdgeEnable, SIGNAL(released()), this, SLOT(collectEdgeDetectParameters()));
    connect(ui->comboBox_Edge, SIGNAL(currentIndexChanged(int)), this, SLOT(adjustEdgeSliderRange(int)));
    connect(ui->comboBox_Edge, SIGNAL(currentIndexChanged(int)), this, SLOT(collectEdgeDetectParameters()));
    connect(edgeFocusFilter, SIGNAL(focusDetected(bool)), ui->radioButton_EdgeEnable, SLOT(setChecked(bool)));
    connect(edgeFocusFilter, SIGNAL(focusDetected(bool)), this, SLOT(changeSampleImage(bool)));
    connect(ui->horizontalSlider_EdgeWeight, SIGNAL(valueChanged(int)), this, SLOT(collectEdgeDetectParameters()));
    connect(ui->radioButton_EdgeEnable, SIGNAL(toggled(bool)), this, SLOT(changeSampleImage(bool)));

}

// destructor
FilterMenu::~FilterMenu()
{
    //end worker thread once event loop finishes
    if(filterWorker_m)
    {
        worker_m.terminate();
        worker_m.wait();
        delete filterWorker_m;
        filterWorker_m = nullptr;
    }
    delete ui;
}

/* This slot is used to update the member addresses for the master and preview images stored
 * in the parent object. If the Mat's become empty in the parent object this slot
 * should be signaled with nullptrs to signify they are empty. */
void FilterMenu::receiveImageAddresses(const cv::Mat *masterImage, cv::Mat *previewImage)
{
    masterImage_m = masterImage;
    previewImage_m = previewImage;
    qDebug() << "Filter Menu Images:" << masterImage_m << previewImage_m;
    emit distributeImageBufferAddresses(masterImage, previewImage);
}

// Enables or disables tracking for the appropriate menu widgets
void FilterMenu::setMenuTracking(bool enable)
{
    ui->horizontalSlider_SmoothWeight->setTracking(enable);
    ui->horizontalSlider_SharpenWeight->setTracking(enable);
    ui->horizontalSlider_EdgeWeight->setTracking(enable);
}

// Function initializes the necessary widget values to their starting values.
void FilterMenu::initializeSliders()
{
    ui->radioButton_SmoothEnable->blockSignals(true);
    ui->radioButton_SharpenEnable->blockSignals(true);
    ui->radioButton_EdgeEnable->blockSignals(true);
    ui->horizontalSlider_SmoothWeight->blockSignals(true);
    ui->horizontalSlider_SharpenWeight->blockSignals(true);
    ui->horizontalSlider_EdgeWeight->blockSignals(true);
    ui->comboBox_Smooth->blockSignals(true);
    ui->comboBox_Sharpen->blockSignals(true);
    ui->comboBox_Edge->blockSignals(true);

    //reinitialize buttons to unchecked
    if(buttonGroup_m->checkedButton() != nullptr)
    {
        buttonGroup_m->setExclusive(false);
        buttonGroup_m->checkedButton()->setChecked(false);
        buttonGroup_m->setExclusive(true);
    }

    ui->horizontalSlider_SmoothWeight->setValue(ui->horizontalSlider_SmoothWeight->minimum());
    ui->horizontalSlider_SharpenWeight->setValue(ui->horizontalSlider_SharpenWeight->minimum());
    ui->horizontalSlider_EdgeWeight->setValue(ui->horizontalSlider_EdgeWeight->minimum());
    ui->label_SampleImage->setPixmap(QPixmap::fromImage(QImage(":/img/icons/masterIcons/rgb.png")));

    ui->radioButton_SmoothEnable->blockSignals(false);
    ui->radioButton_SharpenEnable->blockSignals(false);
    ui->radioButton_EdgeEnable->blockSignals(false);
    ui->horizontalSlider_SmoothWeight->blockSignals(false);
    ui->horizontalSlider_SharpenWeight->blockSignals(false);
    ui->horizontalSlider_EdgeWeight->blockSignals(false);
    ui->comboBox_Smooth->blockSignals(false);
    ui->comboBox_Sharpen->blockSignals(false);
    ui->comboBox_Edge->blockSignals(false);
}

//Changes the slider range for the SharpenSlider based on the needs of the filter selected from the combo box.
void FilterMenu::adjustSharpenSliderRange(int value)
{
    //if not default filter (unsharpen), switch to maximum value of 50 for no gap in slider values
    if(value == FilterLaplacian)
    {
        ui->horizontalSlider_SharpenWeight->setValue(ui->horizontalSlider_SharpenWeight->value() / 2);
        ui->horizontalSlider_SharpenWeight->setMaximum(50);
        ui->horizontalSlider_SharpenWeight->setPageStep(2);
    }
    else
    {
        ui->horizontalSlider_SharpenWeight->setMaximum(100);
        ui->horizontalSlider_SharpenWeight->setValue(ui->horizontalSlider_SharpenWeight->value() * 2);
        ui->horizontalSlider_SharpenWeight->setPageStep(3);
    }
}

//Changes the slider range for the EdgeSlider based on the needs of the filter selected from the combo box.
void FilterMenu::adjustEdgeSliderRange(int value)
{
    //if canny filter, switch to minimum value to 1 (1*2+1=3) in passing slot for no gap in slider values
    if(value == FilterCanny)
    {
        ui->horizontalSlider_EdgeWeight->setValue(1);
        ui->horizontalSlider_EdgeWeight->setMinimum(1);
    }
    else
    {
        ui->horizontalSlider_EdgeWeight->setMinimum(0);
    }
}

//Populates the menuValues_m parameter and passes it to a worker slot for the Smooth operation.
void FilterMenu::collectBlurParameters()
{
    //if filter not enabled, do nothing
    if(!ui->radioButton_SmoothEnable->isChecked()) return;

    menuValues_m[KernelType] = ui->comboBox_Smooth->currentIndex();
    menuValues_m[KernelWeight] = ui->horizontalSlider_SmoothWeight->value();
    menuValues_m[KernelOperation] = SmoothFilter;

    workSignalSuppressor.receiveNewData(QByteArray(reinterpret_cast<char*>(&menuValues_m), sizeof(int) * 3));
}

//Populates the menuValues_m parameter and passes it to a worker slot for the Sharpen operation.
void FilterMenu::collectSharpenParameters()
{
    //if filter not enabled, do nothing
    if(!ui->radioButton_SharpenEnable->isChecked()) return;

    menuValues_m[KernelType] = ui->comboBox_Sharpen->currentIndex();
    menuValues_m[KernelWeight] = ui->horizontalSlider_SharpenWeight->value();
    menuValues_m[KernelOperation] = SharpenFilter;

    workSignalSuppressor.receiveNewData(QByteArray(reinterpret_cast<char*>(&menuValues_m), sizeof(int) * 3));
}

//Populates the menuValues_m parameter and passes it to a worker slot for the Edge Detect operation.
void FilterMenu::collectEdgeDetectParameters()
{
    //if filter not enabled, do nothing
    if(!ui->radioButton_EdgeEnable->isChecked()) return;

    //sends values of 1/3/5/7 for opencv functions. Slider ranges from 0 to 3
    menuValues_m[KernelType] = ui->comboBox_Edge->currentIndex();
    menuValues_m[KernelWeight] = ui->horizontalSlider_EdgeWeight->value() * 2 + 1;
    menuValues_m[KernelOperation] = EdgeFilter;

    workSignalSuppressor.receiveNewData(QByteArray(reinterpret_cast<char*>(&menuValues_m), sizeof(int) * 3));
}

//Sets the sample image based on the menu item selected.
void FilterMenu::changeSampleImage(bool detected)
{
    if(detected)
    {
        if(ui->radioButton_SmoothEnable->isChecked())
            ui->label_SampleImage->setPixmap(QPixmap::fromImage(QImage(":/img/icons/filterMenu/blur.png")));
        else if(ui->radioButton_SharpenEnable->isChecked())
            ui->label_SampleImage->setPixmap(QPixmap::fromImage(QImage(":/img/icons/filterMenu/sharp.png")));
        else if(ui->radioButton_EdgeEnable->isChecked())
            ui->label_SampleImage->setPixmap(QPixmap::fromImage(QImage(":/img/icons/filterMenu/edge.png")));
    }
}

//overloads setVisible to signal the worker thread to cancel any adjustments that weren't applied when minimized
void FilterMenu::setVisible(bool visible)
{
    manageWorker(visible);
    QWidget::setVisible(visible);
}

//overloads show event to initialize the visible menu widgets before being seen
void FilterMenu::showEvent(QShowEvent *event)
{
    initializeSliders();
    QWidget::showEvent(event);
}

/* This method determines when the worker thread should be created or destroyed so
 * that the worker thread (with event loop) is only running if it is required (in
 * this case if the menu is visible). This thread manages the creation, destruction,
 * connection, and disconnection of the thread and its signals / slots.*/
void FilterMenu::manageWorker(bool life)
{
    if(life)
    {
        if(!filterWorker_m)
        {
            //If worker is still trying to exit, wait and process other events until its done
            if(worker_m.isRunning())
            {
                QApplication::setOverrideCursor(Qt::WaitCursor);
                qDebug() << "Waiting for thread to exit";
                while(!worker_m.isFinished())
                {
                    QApplication::processEvents(QEventLoop::AllEvents, 100);
                }
                QApplication::restoreOverrideCursor();
            }

            filterWorker_m = new FilterWorker(masterImage_m, previewImage_m, workerMutex_m);
            filterWorker_m->moveToThread(&worker_m);
            //signal slot connections (might be able to do them in constructor?)
            connect(this, SIGNAL(distributeImageBufferAddresses(const cv::Mat*,cv::Mat*)), filterWorker_m, SLOT(receiveImageAddresses(const cv::Mat*, cv::Mat*)));
            connect(&workSignalSuppressor, SIGNAL(suppressedSignal(SignalSuppressor*)), filterWorker_m, SLOT(receiveSuppressedSignal(SignalSuppressor*)));
            connect(filterWorker_m, SIGNAL(updateDisplayedImage()), this, SIGNAL(updateDisplayedImage()));
            connect(filterWorker_m, SIGNAL(updateStatus(QString)), this, SIGNAL(updateStatus(QString)));
            worker_m.start();
        }
    }
    else
    {
        //while the worker event loop is running, tell it to delete itself once loop is empty.
        if(filterWorker_m)
        {
            /* All signals to and from the object are automatically disconnected (string based, not functor),
             * and any pending posted events for the object are removed from the event queue. This is done incase functor signal/slots used later*/
            disconnect(this, SIGNAL(distributeImageBufferAddresses(const cv::Mat*,cv::Mat*)), filterWorker_m, SLOT(receiveImageAddresses(const cv::Mat*, cv::Mat*)));
            disconnect(&workSignalSuppressor, SIGNAL(suppressedSignal(SignalSuppressor*)), filterWorker_m, SLOT(receiveSuppressedSignal(SignalSuppressor*)));
            disconnect(filterWorker_m, SIGNAL(updateDisplayedImage()), this, SIGNAL(updateDisplayedImage()));
            disconnect(filterWorker_m, SIGNAL(updateStatus(QString)), this, SIGNAL(updateStatus(QString)));
            filterWorker_m->deleteLater();
            filterWorker_m = nullptr;
            worker_m.quit();
        }
    }
}
