/***********************************************************************
* FILENAME :    filterworker.cpp
*
* LICENSE:
*       qcvTouchUp provides an image processing toolset for editing
*       photographs, purposed and packaged for use in a desktop application
*       user environment. Copyright (C) 2018,  Matthew R. Miller
*
*       This program is free software: you can redistribute it and/or modify
*       it under the terms of the GNU General Public License as published by
*       the Free Software Foundation (version 3 of the License) and the
*       3-clause BSD License as agreed upon through the use of the Qt toolkit
*       and OpenCV libraries in qcvTouchUp development, respectively. Copies
*       of the appropriate license files for qcvTouchup, and its source code,
*       can be found in LICENSE.Qt.txt and LICENSE.CV.txt.
*
*       This program is distributed in the hope that it will be useful,
*       but WITHOUT ANY WARRANTY; without even the implied warranty of
*       MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*       GNU General Public License for more details.
*
*       You should have received a copy of the GNU General Public License and
*       3-clause BSD License along with this program.  If not, please see
*       <http://www.gnu.org/licenses/> and <https://opencv.org/license.html>.
*
*       If you wish to contact the developer about this project, please do so
*       through their account at <https://github.com/mattrussmill>
*
* DESCRIPTION :
*       This is the worker thread object tied to the filtermenu.cpp object.
*       The worker performs the operations for filtering the image such as
*       sharpening, bluring, and edge detect. It collects all necessary slider
*       values in a QVarient and copies the object through the signal/signal
*       mechanism to provide the necessary parameters for image processing.
*
* NOTES :
*       This worker thread does not use the OpenCL enhancements offered implicitly
*       through the UMat OpenCV object. The filtering operations had bugs associated
*       with them. This may be bugs within the OpenCV Version used (3.3.1) but I think
*       these are hardware accelerated implicitly anyway with OpenCV 3+ ?
*
* AUTHOR :  Matthew R. Miller       START DATE :    March 03/08/2019
*
* CHANGES : N/A - N/A
*
* VERSION       DATE            WHO                     DETAIL
* 0.1           03/08/2019      Matthew R. Miller       Initial Rev
*
************************************************************************/

#include "filterworker.h"
#include "signalsuppressor.h"
#include "filtermenu.h"
#include <QMutex>
#include <QString>
#include <opencv2/imgproc.hpp>
#include <opencv2/core/ocl.hpp>
#include <QDebug>

/* Constructor initializes the appropriate member variables for the worker object. */
FilterWorker::FilterWorker(const cv::Mat *masterImage, cv::Mat *previewImage, QMutex *mutex, QObject *parent)
    : QObject(parent)
{
    emit updateStatus("Filter Menu initializing...");

    mutex_m = mutex;
    masterImage_m = masterImage;
    previewImage_m = previewImage;
    qDebug() << "Filter Worker Created! - Images:" << masterImage_m << previewImage_m;

    emit updateStatus("");

}

// destructor
FilterWorker::~FilterWorker()
{
    qDebug() << "FilterWorker destroyed";
}

/* This member (slot) recieves the data from the controlling class (slow thread). The data
 * is sent as a pointer to the class itself who's member contains the data. To see how this
 * works see signalsuppressor.h/cpp. The format is tied to the associated menu object. */
void FilterWorker::receiveSuppressedSignal(SignalSuppressor *dataContainer)
{
    data_m = dataContainer->getNewData().toByteArray();
    int *parameters = reinterpret_cast<int*>(data_m.data());
    performImageFiltering(parameters);
    emit updateDisplayedImage();
}

/* This slot is used to update the member addresses for the master and preview images stored
 * in the controlling thread. If the Mat's become empty in the controlling thread this slot
 * should be signaled with nullptrs to signify they are empty. */
void FilterWorker::receiveImageAddresses(const cv::Mat *masterImage, cv::Mat *previewImage)
{
    masterImage_m = masterImage;
    previewImage_m = previewImage;
    qDebug() << "Filter Worker Images:" << masterImage_m << previewImage_m;
}

///////////////////////////////--- Filter Menu Computations ---///////////////////////////////
/* Sets the kernel radius for a filter to a maximum of 0.015 times the smallest image dimension.
 * The kernel size is then scaled between 1 and 100% of its maximum size through weightPercent.
 * The result of this function must also always be odd. */
int FilterWorker::kernelSize(QSize image, int weightPercent)
{
    int ksize;
    if(image.width() > image.height())
        ksize = image.height();
    else
        ksize = image.width();

    if(weightPercent > 100)
        weightPercent = 100;
    else if(weightPercent < 1)
        weightPercent = 1;

    ksize *= 0.015 * (weightPercent / 100.0);

    return ksize | 1;
}

/* Generates a 2D Laplacian kernel for use with OpenCV's Filter2D function. If the desired size is
 * negative, the kernel will output a Mat with one element of 1. If passed an even size, the next
 * greatest odd size is used.*/
cv::Mat FilterWorker::makeLaplacianKernel(int size)
{
    if(size < 1)
        size = 1;
    size |= 1; //must be odd
    int matCenter = size >> 1;

    //fill new kernel with zeroes
    cv::Mat newKernel = cv::Mat::zeros(size, size, CV_32F);

    //fill matrix from center; traverse approx 1/4 elements
    int kernelPoint;
    int kernelSum = 0;
    for(int i = 0; i < matCenter + 1; i++)
    {
        for(int j = 0; j < matCenter + 1; j++)
        {
            kernelPoint = -(1 + i + j - matCenter);
            if (kernelPoint > 0) kernelPoint = 0;

            //top left
            newKernel.at<float>(cv::Point(i, j)) = kernelPoint;

            //bottom right
            newKernel.at<float>(cv::Point(size - i - 1, size - j - 1)) = kernelPoint;

            //do not write & count multiple times and to sum properly
            if(i != size >> 1 && j != size >> 1)
            {
                kernelSum += kernelPoint * 4;

                //top right
                newKernel.at<float>(cv::Point(size - i - 1, j)) = kernelPoint;

                //bottom left
                newKernel.at<float>(cv::Point(i, size - j - 1)) = kernelPoint;
            }
            else
            {
                kernelSum += kernelPoint * 2;
            }
        }
    }

    //adjust the kernel sum to exclude the center point. Invert and set as center.
    kernelSum -= newKernel.at<float>(cv::Point(matCenter, matCenter)) * 2;
    newKernel.at<float>(cv::Point(matCenter, matCenter)) = -kernelSum;
    return newKernel;
}

/* Performs the smoothing, sharpening, and edge detection operations from the Filter menu
 * in the GUI. Switch statement selects the type of smoothing that will be applied to the
 * image in the master buffer. The parameter array passes all the necessary parameters to
 * the worker thread based on the openCV functions it calls.*/
void FilterWorker::performImageFiltering(int *parameter)
{
    emit updateStatus("Working...");
    if(mutex_m) mutex_m->lock();
    if(masterImage_m == nullptr || previewImage_m == nullptr)
    {
        if(mutex_m) mutex_m->unlock();
        qDebug() << "Cannot perform Adjustments, image not attached";
        return;
    }

    switch (parameter[FilterMenu::KernelOperation])
    {

    case FilterMenu::SmoothFilter:
    {
        int ksize = kernelSize(QSize(masterImage_m->cols, masterImage_m->rows), parameter[FilterMenu::KernelWeight]);

        switch (parameter[FilterMenu::KernelType])
        {

        case FilterMenu::FilterGaussian:
        {
            //For Gaussian, sigma should be 1/4 size of kernel. (HAS GLITCH WITH UMAT OUTPUT)
            cv::GaussianBlur(*masterImage_m, *previewImage_m, cv::Size(ksize, ksize), ksize * 0.25);
            qDebug() << "Filter Gaussian" << ksize;
            break;
        }
        case FilterMenu::FilterMedian:
        {
            cv::medianBlur(*masterImage_m, *previewImage_m, ksize);
            qDebug() << "Filter Median" << ksize;
            break;
        }
        default: //FilterMenu::FilterAverage
        {
            cv::blur(*masterImage_m, *previewImage_m, cv::Size(ksize, ksize));
            qDebug() << "Filter Average" << ksize;
            break;
        }
        }
        break;
    }
    case FilterMenu::SharpenFilter:
    {
        int ksize = kernelSize(QSize(masterImage_m->cols, masterImage_m->rows), parameter[FilterMenu::KernelWeight]);

        switch (parameter[FilterMenu::KernelType])
        {

        case FilterMenu::FilterLaplacian:
        {
            //blur first to reduce noise
            cv::GaussianBlur(*masterImage_m, *previewImage_m, cv::Size(3, 3), 0);
            cv::filter2D(*previewImage_m, *previewImage_m, CV_8U,
                         makeLaplacianKernel(parameter[FilterMenu::KernelWeight]));
            cv::addWeighted(*masterImage_m, .9, *previewImage_m, .1, 255 * 0.1, *previewImage_m, masterImage_m->depth());
            break;
        }
        default: //FilterMenu::FilterUnsharpen
        {
            cv::GaussianBlur(*masterImage_m, tmpImage_m, cv::Size(ksize, ksize), ksize * 0.25);
            cv::addWeighted(*masterImage_m, 1.5, tmpImage_m, -0.5, 0, *previewImage_m, masterImage_m->depth());
            break;
        }
        }
        break;
    }
    case FilterMenu::EdgeFilter:
    {
        switch (parameter[FilterMenu::KernelType])
        {

        //these opencv functions can have aperature size of 1/3/5/7
        case FilterMenu::FilterLaplacian:
        {
            cv::Laplacian(*masterImage_m, *previewImage_m, CV_8U, parameter[FilterMenu::KernelWeight]);
            break;
        }

        case FilterMenu::FilterSobel:
        {
            cv::Sobel(*masterImage_m, tmpImage_m, CV_8U, 1, 0, parameter[FilterMenu::KernelWeight]);
            cv::Sobel(*masterImage_m, *previewImage_m, CV_8U, 0, 1, parameter[FilterMenu::KernelWeight]);
            cv::addWeighted(tmpImage_m, 0.5, *previewImage_m, 0.5, 0, *previewImage_m, masterImage_m->depth());
            break;
        }

        default: //FilterMenu::FilterCanny
        {
            cv::Canny(*masterImage_m, *previewImage_m, 80, 200, parameter[FilterMenu::KernelWeight]);
            qDebug() << "channels:" << QString::number(masterImage_m->channels());
            break;
        }
        }
        break;
    }
    default:
        qDebug() << "NO FILTERING OCCURED";
        break;
    }

    //after computation is complete, push image and histogram to GUI if changes were made
    if(mutex_m) mutex_m->unlock();
    emit updateStatus("");
}
