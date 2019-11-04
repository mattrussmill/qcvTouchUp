/***********************************************************************
* FILENAME :    temperatureworker.cpp
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
*       This is the worker thread object tied to the temperaturemenu.cpp object.
*       The worker performs the operations for adjusting the lighting temperature.
*       It transmits the temperature value in a QVarient and copies the object
*       through the signal/signal mechanism to provide the necessary parameters
*       for image processing.
*
* NOTES :
*       This worker thread uses OpenCV OpenCL accelerated function calls implicitly
*       when OpenCL hardware is available through OpenCV's UMat object calls. In the
*       constructor there is an OpenCL initialization step where the OpenCL commands
*       are given before the functionality is actually used through calling the
*       appropriate performImageAdjustments method prematurely. See Issue #41 for
*       more detail.
*
*
* AUTHOR :  Matthew R. Miller       START DATE :    March 03/04/2019
*
* CHANGES : N/A - N/A
*
* VERSION       DATE            WHO                     DETAIL
* 0.1           03/04/2019      Matthew R. Miller       Initial Rev
*
************************************************************************/
#include "temperatureworker.h"
#include "signalsuppressor.h"
#include <QMutex>
#include <QString>
#include <opencv2/imgproc.hpp>
#include <opencv2/core/ocl.hpp>
#include <QDebug>

TemperatureWorker::TemperatureWorker(const cv::Mat *masterImage, cv::Mat *previewImage, QMutex *mutex, QObject *parent)
    : QObject(parent)
{
    emit updateStatus("Temperature Menu initializing...");

    //OpenCL initialization step to build the OpenCL calls in GPU before the worker is called with an attached image
    cv::ocl::Context ctx = cv::ocl::Context::getDefault();
    if (ctx.ptr())
    {
        cv::Mat tmpMat(100, 100, CV_8UC3);
        cv::randu(tmpMat, cv::Scalar(0, 0, 0), cv::Scalar(255, 255, 255));
        int tmpParameters = 0;

        mutex_m = nullptr;
        masterImage_m = &tmpMat;
        previewImage_m = &tmpMat;

        performLampTemperatureShift(tmpParameters);
    }

    mutex_m = mutex;
    masterImage_m = masterImage;
    previewImage_m = previewImage;
    qDebug() << "Temperature Worker Created! - Images:" << masterImage_m << previewImage_m;

    emit updateStatus("");
}

TemperatureWorker::~TemperatureWorker()
{
    qDebug() << "TemperatureWorker destroyed";
}

/* This member (slot) recieves the data from the controlling class (slow thread). The data
 * is sent as a pointer to the class itself who's member contains the data. To see how this
 * works see signalsuppressor.h/cpp. The format is tied to the associated menu object. */
void TemperatureWorker::receiveSuppressedSignal(SignalSuppressor *dataContainer)
{
    data_m = dataContainer->getNewData().toInt();
    performLampTemperatureShift(data_m);
    emit updateDisplayedImage();
}

/* This slot is used to update the member addresses for the master and preview images stored
 * in the controlling thread. If the Mat's become empty in the controlling thread this slot
 * should be signaled with nullptrs to signify they are empty. */
void TemperatureWorker::receiveImageAddresses(const cv::Mat *masterImage, cv::Mat *previewImage)
{
    masterImage_m = masterImage;
    previewImage_m = previewImage;
    qDebug() << "Adjust Worker Images:" << masterImage_m << previewImage_m;
}


void TemperatureWorker::performLampTemperatureShift(int parameter)
{
    emit updateStatus("Working...");
    if(mutex_m) mutex_m->lock();
    if(masterImage_m == nullptr || previewImage_m == nullptr)
    {
        if(mutex_m) mutex_m->unlock();
        qDebug() << "Cannot perform Adjustments, image not attached";
        emit updateStatus("");
        return;
    }

    //clone necessary because internal checks will prevent GUI image from cycling.
    masterImage_m->copyTo(implicitOclImage_m);

    float yred, ygreen, yblue;
    double x = parameter / 100.0;

    //step function for red and green channels
    if(parameter > 6500)
    {
        yred = 479.7143 - 4.757143 * x + 0.02 * pow(x, 2);
        ygreen = 369.3214 - 2.502381 * x + 0.01 * pow(x, 2);
    }
    else
    {
        yred = 255.0;
        ygreen = -50.34577 + 13.21698 * x - 0.2250017 * pow(x, 2.0) + 0.001430717 * pow(x, 3.0);
    }

    //step function for blue channel
    if(parameter < 1500)
    {
        yblue = 0.0;
    }
    else if(parameter > 7000)
    {
        yblue = 255.0;
    }
    else
    {
        yblue = 260.1294 - 51.0587 * x + 3.321611 * pow(x, 2.0) - 0.08684615 * pow(x, 3.0)
                + 0.001048834 * pow(x, 4.0) - 0.000004820513 * pow(x, 5.0);
    }

    //split each channel and manipulate each channel individually
    cv::split(implicitOclImage_m, splitChannelsTmp_m);
    splitChannelsTmp_m.at(0) = splitChannelsTmp_m.at(0).mul(yred / 255);
    splitChannelsTmp_m.at(1) = splitChannelsTmp_m.at(1).mul(ygreen / 255);
    splitChannelsTmp_m.at(2) = splitChannelsTmp_m.at(2).mul(yblue / 255);
    cv::merge(splitChannelsTmp_m, implicitOclImage_m);

    implicitOclImage_m.copyTo(*previewImage_m);

    //after computation is complete, push image and histogram to GUI if changes were made
    if(mutex_m) mutex_m->unlock();
    emit updateStatus("");
}
