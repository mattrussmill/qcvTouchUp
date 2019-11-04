/***********************************************************************
* FILENAME :    adjustworker.cpp
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
*       This is the worker thread object tied to the adjustmenu.cpp object.
*       The worker performs the operations for adjusting the color, brightness,
*       contrast, pixel depth, hue, saturation, intensity including high and low
*       adjustments to gamma correction. It collects all necessary slider
*       values in a QVarient and copies the object through the signal/signal
*       mechanism to provide the necessary parameters for image processing.
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
* 0.2           09/04/2019      Matthew R. Miller       Depth Fixed
*
************************************************************************/

#include "adjustworker.h"
#include "signalsuppressor.h"
#include "adjustmenu.h"
#include <QMutex>
#include <QString>
#include <opencv2/imgproc.hpp>
#include <opencv2/core/ocl.hpp>
#include <QDebug>
//#include <QElapsedTimer>

#define HUE_DEPTH_SEPARATION 30
#define INTENSITY_DEPTH_SEPARATION 128
#define SATURATION_DEPTH_SEPARATION INTENSITY_DEPTH_SEPARATION

/* Constructor initializes the appropriate member variables for the worker object. If
 * an OpenCL device is detected as available on the system, a pre-initialization step is
 * performed to increase initial performance. */
AdjustWorker::AdjustWorker(const cv::Mat *masterImage, cv::Mat *previewImage, QMutex *mutex, QObject *parent)
    : QObject(parent)
{
    emit updateStatus("Adjust Menu initializing...");
    //QElapsedTimer timer;
    //timer.start();
    lookUpTable_m = cv::Mat(1, 256, CV_8U);

    //OpenCL initialization step to build the OpenCL calls in GPU before the worker is called with an attached image
    cv::ocl::Context ctx = cv::ocl::Context::getDefault();
    if (ctx.ptr())
    {
        cv::Mat tmpMat(100, 100, CV_8UC3);
        cv::randu(tmpMat, cv::Scalar(0, 0, 0), cv::Scalar(255, 255, 255));
        float tmpParameters[] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

        mutex_m = nullptr;
        masterImage_m = &tmpMat;
        previewImage_m = &tmpMat;

        performImageAdjustments(tmpParameters);
    }

    mutex_m = mutex;
    masterImage_m = masterImage;
    previewImage_m = previewImage;
    qDebug() << "Adjust Worker Created! - Images:" << masterImage_m << previewImage_m;

    //qDebug() << "The slow operation took" << timer.elapsed() << "milliseconds";
    emit updateStatus("");
}

// destructor
AdjustWorker::~AdjustWorker()
{
    qDebug() << "AdjustWorker destroyed";
}

/* This member (slot) recieves the data from the controlling class (slow thread). The data
 * is sent as a pointer to the class itself who's member contains the data. To see how this
 * works see signalsuppressor.h/cpp. The format is tied to the associated menu object. */
void AdjustWorker::receiveSuppressedSignal(SignalSuppressor *dataContainer)
{
    data_m = dataContainer->getNewData().toByteArray();
    float *parameters = reinterpret_cast<float*>(data_m.data());
    performImageAdjustments(parameters);
    emit updateDisplayedImage();
}

/* This slot is used to update the member addresses for the master and preview images stored
 * in the controlling thread. If the Mat's become empty in the controlling thread this slot
 * should be signaled with nullptrs to signify they are empty. */
void AdjustWorker::receiveImageAddresses(const cv::Mat *masterImage, cv::Mat *previewImage)
{
    masterImage_m = masterImage;
    previewImage_m = previewImage;
    qDebug() << "Adjust Worker Images:" << masterImage_m << previewImage_m;
}

/* Performs the image adjustment operations from the Adjust menu in the GUI. If the images
 * exist in memory the function locks the mutex and copies the necessary parameters before
 * performing the desired operations only for the corresponding sliders in the .ui file that
 * have changed from their default value. Using a QVector forces a copy when passing information*/
void AdjustWorker::performImageAdjustments(float * parameter)
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

    //--perform operations on hue, intensity, and saturation color space if values are not set to initial
    if(parameter[AdjustMenu::Hue] != 0.0f || parameter[AdjustMenu::Intensity] != 0.0f
            || parameter[AdjustMenu::Saturation] != 0.0f || parameter[AdjustMenu::Gamma] != 1.0f
            || parameter[AdjustMenu::Highlight] != 0.0f || parameter[AdjustMenu::Shadows] != 0.0f
            || parameter[AdjustMenu::Depth] < 255)
    {
        cv::cvtColor(implicitOclImage_m, implicitOclImage_m, cv::COLOR_RGB2HLS);
        cv::split(implicitOclImage_m, splitChannelsTmp_m);

        /* openCv hue is stored as 360/2 since uchar cannot store above 255 so a LUT is populated
             * from 0 to 180 and phase shifted between -180 and 180 based on slider input. */
        if(parameter[AdjustMenu::Hue] != 0.0f)
        {
            //populate LUT to map the current values to correct phase (only 180 cells used)
            int hueShifted;
            cv::Mat lookUpTable(1, 256, CV_8UC1);
            for(int i = 0; i < 180; i++)
            {
                hueShifted = i + parameter[AdjustMenu::Hue];
                if(hueShifted < 0)
                    hueShifted += 180;
                else if(hueShifted > 179)
                    hueShifted -=180;
                lookUpTable.data[i] = hueShifted;
            }
            cv::LUT(splitChannelsTmp_m.at(0), lookUpTable, splitChannelsTmp_m[0]);
        }

        //adjust the intensity
        if(parameter[AdjustMenu::Intensity] != 0.0f)
            splitChannelsTmp_m.at(1).convertTo(splitChannelsTmp_m[1], -1, 1.0, parameter[AdjustMenu::Intensity]);

        //adjust the saturation
        if(parameter[AdjustMenu::Saturation] != 0.0f)
            splitChannelsTmp_m.at(2).convertTo(splitChannelsTmp_m[2], -1, 1.0, parameter[AdjustMenu::Saturation]);

        //adjust gamma by 255(i/255)^(1/gamma) where gamma 0.5 to 3.0
        if(parameter[AdjustMenu::Gamma] != 1.0f || parameter[AdjustMenu::Highlight] != 0.0f
                || parameter[AdjustMenu::Shadows] != 0.0f)
        {
            //fill LUT for gamma adjustment
            float tmpGamma;
            for(int i = 0; i < 256; i++)
            {
                //adjust gamma
                tmpGamma = 255.0f * powf(i / 255.0f, 1.0f / parameter[AdjustMenu::Gamma]);

                /* Bound the shadow adjustment to all pixels below 149 such that the x axis is not
                     * crossed (output is not fliped) in the adjustment equation -(x/50.0)^4 + x and is
                     * handled as a step function. Function is deisned to taper towards zero as the bound
                     * is approached in conjunction with the highlight adjustment. Equation inverted for
                     * subtraction. Shadow operates on the gamma adjusted LUT from a range of -80 to 80*/
                if(i < 149 && parameter[AdjustMenu::Shadows] != 0.0f)
                {
                    float tmpShadow;
                    if(parameter[AdjustMenu::Shadows] > 0.0f)
                    {
                        //-(x/50.0)^4 + shadow
                        tmpShadow = -1 * powf(i / 50.0f, 4) + parameter[AdjustMenu::Shadows];
                        if(tmpShadow > 0)
                            tmpGamma += tmpShadow;
                    }
                    else
                    {
                        //(x/50.0)^4 - shadow .. (shadow is negative)
                        tmpShadow = powf(i / 50.0f, 4) + parameter[AdjustMenu::Shadows];
                        if(tmpShadow < 0)
                            tmpGamma += tmpShadow;
                    }
                }

                /* Bound the highlight adjustment to all pixels above 106 such that the x axis is not
                     * crossed (output is not fliped) in the adjustment equation -(x/50.0 - 5.1)^4 + x and is
                     * handled as a step function. Function is deisned to taper towards zero as the bound
                     * is approached in conjunction with the highlight adjustment.Equation inverted for
                     * subtraction. Highlight operates on the gamma adjusted LUT from a range of -80 to 80*/
                if(i > 106 && parameter[AdjustMenu::Highlight] != 0.0f)
                {
                    float tmpHighlight;
                    if(parameter[AdjustMenu::Highlight] > 0.0f)
                    {
                        //-(x/50.0 - 5.1)^4 + highlight
                        tmpHighlight = -1 * powf(i / 50.0f - 5.1f, 4) + parameter[AdjustMenu::Highlight];
                        if(tmpHighlight > 0)
                            tmpGamma += tmpHighlight;
                    }
                    else
                    {
                        //(x/50.0 - 5.1)^4 - highlight .. (highlight is negative)
                        tmpHighlight = powf(i / 50.0f - 5.1f, 4) + parameter[AdjustMenu::Highlight];
                        if(tmpHighlight < 0)
                            tmpGamma += tmpHighlight;
                    }
                }

                //limit gamma adjusted values
                if(tmpGamma > 255)
                    tmpGamma = 255;
                else if(tmpGamma < 0)
                    tmpGamma = 0;

                lookUpTable_m.data[i] = tmpGamma;
            }

            //replace pixel values based on their LUT value
            cv::LUT(splitChannelsTmp_m.at(1), lookUpTable_m, splitChannelsTmp_m[1]);
        }

        //--adjust the number of colors available of not at initial value of 255
        if(parameter[AdjustMenu::Depth] < 255)
        {
            //create and normalize LUT for 0 to 180 for Hue; replace pixel intensities based on their LUT value
            float scaleFactor = 1.0f - (parameter[AdjustMenu::Depth] * (180.0f / 255.0f)) / 180.0f;
            int mod;
            for(int i = 0; i < 180; i++)
            {
                /*if color is closer to the lower hue separation level than the higher hue separation level in radians
                  (scaled by half for OpenCV - 180 not 360), scale towards the lower level, else scale towards the higher level */
                mod = (i + 1) % HUE_DEPTH_SEPARATION;
                if( mod <= HUE_DEPTH_SEPARATION / 2)
                {
                    lookUpTable_m.data[i] = static_cast<uchar>(static_cast<float>(i + 1) - roundf(mod * scaleFactor));
                    //qDebug() << "Data %30 =< 15 : " << lookUpTable_m.data[i];
                }
                else
                {
                    lookUpTable_m.data[i] = static_cast<uchar>(static_cast<float>(i + 1) + roundf(((HUE_DEPTH_SEPARATION - mod) * scaleFactor) ));
                    //qDebug() << "Data %30 > 15 : " << lookUpTable_m.data[i];
                }
            }
            cv::LUT(splitChannelsTmp_m.at(0), lookUpTable_m, splitChannelsTmp_m[0]);

            //create and normalize LUT from 0 to largest intensity / saturation values, then scale from 0 to 255
            float tmp;
            scaleFactor = 1.0f - (parameter[AdjustMenu::Depth] / 255.0f);
            for(int i = 0; i < 256; i++)
            {
                /*if color is closer to the lower hue separation level than the higher saturation separation level in 255 scaled value;
                  scale towards the lower level, else scale towards the higher level */
                mod = (i + 1) % INTENSITY_DEPTH_SEPARATION;
                if( mod <= INTENSITY_DEPTH_SEPARATION / 2)
                {
                    tmp = static_cast<float>(i + 1) - roundf(mod * scaleFactor);
                    if(tmp > 255.0f)
                        tmp = 255.0f;
                    lookUpTable_m.data[i] = static_cast<uchar>(tmp);
                    //qDebug() << "Data %128 =< 64 : " << lookUpTable_m.data[i];
                }
                else
                {
                    lookUpTable_m.data[i] = static_cast<uchar>(static_cast<float>(i + 1) + floorf(((INTENSITY_DEPTH_SEPARATION - mod) * scaleFactor) ));
                    //qDebug() << "Data %128 > 64 : " << lookUpTable_m.data[i];
                }
            }
            cv::LUT(splitChannelsTmp_m.at(1), lookUpTable_m, splitChannelsTmp_m[1]); //sat
            //cv::LUT(splitChannelsTmp_m.at(2), lookUpTable_m, splitChannelsTmp_m[2]); //int
        }
        cv::merge(splitChannelsTmp_m, implicitOclImage_m);
        cv::cvtColor(implicitOclImage_m, implicitOclImage_m, cv::COLOR_HLS2RGB);
    }


    //--convert from color to grayscale if != 1.0
    if(parameter[AdjustMenu::Color] != 1.0f)
    {
        cv::cvtColor(implicitOclImage_m, splitChannelsTmp_m[0], cv::COLOR_RGB2GRAY);
        splitChannelsTmp_m.at(0).copyTo(splitChannelsTmp_m.at(1));
        splitChannelsTmp_m.at(0).copyTo(splitChannelsTmp_m.at(2));
        cv::merge(splitChannelsTmp_m, implicitOclImage_m);
    }


    //--perform contrast and brightness operation if sliders are not at initial positions
    if (parameter[AdjustMenu::Brightness] != 0.0f || parameter[AdjustMenu::Contrast] != 1.0f)
    {
        float alpha = parameter[AdjustMenu::Contrast];
        float beta = parameter[AdjustMenu::Brightness];

        //calculate brightness correction
        if(alpha >= 1)
            beta += -72.8f * log2f(alpha);
        else
            beta += 127.0f * -log2f(alpha) / sqrtf(1 / alpha);

        //perform contrast computation and prime source buffer
        implicitOclImage_m.convertTo(implicitOclImage_m, -1, alpha, beta);
    }

    implicitOclImage_m.copyTo(*previewImage_m);

    //after computation is complete, push image and histogram to GUI if changes were made
    if(mutex_m) mutex_m->unlock();
    emit updateStatus("");
}
