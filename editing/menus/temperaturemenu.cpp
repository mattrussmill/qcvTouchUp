/***********************************************************************
* FILENAME :    temperaturemenu.cpp
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
*       This menu is used for adjusting the perceived temperature of the image's
*       light source. The menu provides preset options as well as a fine adjustment
*       slider.
*
* NOTES :
*
* AUTHOR :  Matthew R. Miller       START DATE :    April 06/27/2018
*
* CHANGES : N/A - N/A
*
* VERSION       DATE            WHO                     DETAIL
* 0.1           07/02/2018      Matthew R. Miller       Initial Rev
*
************************************************************************/
#include "temperaturemenu.h"
#include "../../app_filters/mousewheeleatereventfilter.h"
#include "ui_temperaturemenu.h"
#include "../workers/temperatureworker.h"
#include <QString>
#include <QRadioButton>
#include <QButtonGroup>
#include <QDebug>

/* Constructor installs the MouseWheelEaterFilter for the slider, groups the buttons together for
 * to easily search for the selected button, and establishes all signals/slots necessary.*/
TemperatureMenu::TemperatureMenu(QMutex *mutex, QWidget *parent) :
    QScrollArea(parent),
    ui(new Ui::TemperatureMenu)
{
    ui->setupUi(this);
    masterImage_m = nullptr;
    previewImage_m = nullptr;
    workerMutex_m = mutex;
    temperatureWorker_m = nullptr;

    MouseWheelEaterEventFilter *wheelFilter = new MouseWheelEaterEventFilter(this);
    ui->horizontalSlider_Temperature->installEventFilter(wheelFilter);

    buttonGroup_m = new QButtonGroup(this);
    buttonGroup_m->addButton(ui->radioButton_1850K);
    buttonGroup_m->addButton(ui->radioButton_2400K);
    buttonGroup_m->addButton(ui->radioButton_2550K);
    buttonGroup_m->addButton(ui->radioButton_2700K);
    buttonGroup_m->addButton(ui->radioButton_3200K);
    buttonGroup_m->addButton(ui->radioButton_4500K);
    buttonGroup_m->addButton(ui->radioButton_5000K);
    buttonGroup_m->addButton(ui->radioButton_6500K);
    buttonGroup_m->addButton(ui->radioButton_7200K);
    buttonGroup_m->addButton(ui->radioButton_10000K);

    connect(ui->radioButton_1850K, SIGNAL(clicked(bool)), this, SLOT(moveSliderToButton(bool)));
    connect(ui->radioButton_2400K, SIGNAL(clicked(bool)), this, SLOT(moveSliderToButton(bool)));
    connect(ui->radioButton_2550K, SIGNAL(clicked(bool)), this, SLOT(moveSliderToButton(bool)));
    connect(ui->radioButton_2700K, SIGNAL(clicked(bool)), this, SLOT(moveSliderToButton(bool)));
    connect(ui->radioButton_3200K, SIGNAL(clicked(bool)), this, SLOT(moveSliderToButton(bool)));
    connect(ui->radioButton_4500K, SIGNAL(clicked(bool)), this, SLOT(moveSliderToButton(bool)));
    connect(ui->radioButton_5000K, SIGNAL(clicked(bool)), this, SLOT(moveSliderToButton(bool)));
    connect(ui->radioButton_6500K, SIGNAL(clicked(bool)), this, SLOT(moveSliderToButton(bool)));
    connect(ui->radioButton_7200K, SIGNAL(clicked(bool)), this, SLOT(moveSliderToButton(bool)));
    connect(ui->radioButton_10000K, SIGNAL(clicked(bool)), this, SLOT(moveSliderToButton(bool)));

    connect(ui->horizontalSlider_Temperature, SIGNAL(sliderReleased()), this, SLOT(deselectRadioButtonFromSlider()));
    connect(ui->horizontalSlider_Temperature, SIGNAL(valueChanged(int)), this, SLOT(sendTemperatureValue(int)));

    initializeSliders();
}

//autogenerated destructor
TemperatureMenu::~TemperatureMenu()
{
    //end worker thread once event loop finishes
    if(temperatureWorker_m)
    {
        worker_m.terminate();
        worker_m.wait();
        delete temperatureWorker_m; //safe? Different way to handle this?
        temperatureWorker_m = nullptr;
    }
    delete ui;
}

/* This slot is used to update the member addresses for the master and preview images stored
 * in the parent object. If the Mat's become empty in the parent object this slot
 * should be signaled with nullptrs to signify they are empty. */
void TemperatureMenu::receiveImageAddresses(const cv::Mat *masterImage, cv::Mat *previewImage)
{
    masterImage_m = masterImage;
    previewImage_m = previewImage;
    qDebug() << "Adjust Menu Images:" << masterImage_m << previewImage_m;
    emit distributeImageBufferAddresses(masterImage, previewImage);
}

// Enables or disables tracking for the appropriate menu widgets
void TemperatureMenu::setMenuTracking(bool enable)
{
    ui->horizontalSlider_Temperature->setTracking(enable);
}

// Function initializes the necessary widget values to their starting values.
void TemperatureMenu::initializeSliders()
{
    ui->horizontalSlider_Temperature->blockSignals(true);
    int middle = (ui->horizontalSlider_Temperature->minimum() + ui->horizontalSlider_Temperature->maximum()) / 2;
    ui->horizontalSlider_Temperature->setValue(middle);
    deselectRadioButtonFromSlider();
    ui->horizontalSlider_Temperature->blockSignals(false);
}

/* If adjustSlider is true (QRadioButton enabled), the function retrieves the sender's name
 * and prunes its name such that only the numeric representation of the temperature the radio
 * button is associated with remains. That value is converted to an integer and moves the
 * temperature slider appropriately.*/
void TemperatureMenu::moveSliderToButton(bool adjustSlider)
{
    if(adjustSlider)
    {
        //retrieve sender name and prune object name to only the number
        QString senderName(qobject_cast<QRadioButton *>(sender())->text());
        senderName.chop(1);
        ui->horizontalSlider_Temperature->setValue(senderName.toInt());
    }
}

/* Function looks for the QRadioButton in the buttonGroup_m that is checked. The checked
 * button in the group is then unchecked by turning group exclusivity on and off. If
 * no buttons are checked, nothing is done.*/
void TemperatureMenu::deselectRadioButtonFromSlider()
{
    QAbstractButton *checkedButton = buttonGroup_m->checkedButton();

    if(checkedButton != nullptr)
    {
        buttonGroup_m->setExclusive(false);
        checkedButton->setChecked(false);
        buttonGroup_m->setExclusive(true);
    }
}

//overloads setVisible to signal the worker thread to cancel any adjustments that weren't applied when minimized
void TemperatureMenu::setVisible(bool visible)
{
    manageWorker(visible);
    QWidget::setVisible(visible);
}

//overloads show event to initialize the visible menu widgets before being seen
void TemperatureMenu::showEvent(QShowEvent *event)
{
    initializeSliders();
    QWidget::showEvent(event);
}

//sends temperature value through signal suppressor
void TemperatureMenu::sendTemperatureValue(int value)
{
    workSignalSuppressor.receiveNewData(value);
}

/* This method determines when the worker thread should be created or destroyed so
 * that the worker thread (with event loop) is only running if it is required (in
 * this case if the menu is visible). This thread manages the creation, destruction,
 * connection, and disconnection of the thread and its signals / slots.*/
void TemperatureMenu::manageWorker(bool life)
{
    if(life)
    {
        if(!temperatureWorker_m)
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

            temperatureWorker_m = new TemperatureWorker(masterImage_m, previewImage_m, workerMutex_m);
            temperatureWorker_m->moveToThread(&worker_m);
            //signal slot connections (might be able to do them in constructor?)
            connect(this, SIGNAL(distributeImageBufferAddresses(const cv::Mat*,cv::Mat*)), temperatureWorker_m, SLOT(receiveImageAddresses(const cv::Mat*, cv::Mat*)));
            connect(&workSignalSuppressor, SIGNAL(suppressedSignal(SignalSuppressor*)), temperatureWorker_m, SLOT(receiveSuppressedSignal(SignalSuppressor*)));
            connect(temperatureWorker_m, SIGNAL(updateDisplayedImage()), this, SIGNAL(updateDisplayedImage()));
            connect(temperatureWorker_m, SIGNAL(updateStatus(QString)), this, SIGNAL(updateStatus(QString)));
            worker_m.start();
        }
    }
    else
    {
        //while the worker event loop is running, tell it to delete itself once loop is empty.
        if(temperatureWorker_m)
        {
            /* All signals to and from the object are automatically disconnected (string based, not functor),
             * and any pending posted events for the object are removed from the event queue. This is done incase functor signal/slots used later*/
            disconnect(this, SIGNAL(distributeImageBufferAddresses(const cv::Mat*,cv::Mat*)), temperatureWorker_m, SLOT(receiveImageAddresses(const cv::Mat*, cv::Mat*)));
            disconnect(&workSignalSuppressor, SIGNAL(suppressedSignal(SignalSuppressor*)), temperatureWorker_m, SLOT(receiveSuppressedSignal(SignalSuppressor*)));
            disconnect(temperatureWorker_m, SIGNAL(updateDisplayedImage()), this, SIGNAL(updateDisplayedImage()));
            disconnect(temperatureWorker_m, SIGNAL(updateStatus(QString)), this, SIGNAL(updateStatus(QString)));
            temperatureWorker_m->deleteLater();
            temperatureWorker_m = nullptr;
            worker_m.quit();
        }
    }

}