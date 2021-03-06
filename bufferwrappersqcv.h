/***********************************************************************
* FILENAME :    bufferwrappersqcv.h
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
*       This file adds wrapping functions for managing data between
*       Qt classes using QImages and OpenCV classes using Mat. Wrapping
*       the image buffers in this way is necessary to use the OpenCV library
*       while also being able to display the images in the Qt GUI through
*       QWidgets.
*
* NOTES :
*       This is not a class, this is a collection of functions.
*
* AUTHOR :  Matthew R. Miller       START DATE :    January 01/11/2018
*
* CHANGES : N/A - N/A
*
* VERSION       DATE            WHO                     DETAIL
* 0.1           01/11/2018      Matthew R. Miller       Initial Rev
*
************************************************************************/
#ifndef BUFFERWRAPPERSQCV_H
#define BUFFERWRAPPERSQCV_H

#include <QString>
#include <QDebug>
#include <opencv2/core.hpp>
class QImage;
class QPixmap;

namespace qcv
{
    using namespace cv;
    QString getMatType(const Mat &image);
    QImage cvMatToQImage(const Mat &image, bool implicitBuffer = true);
    Mat qImageToCvMat(const QImage &image, bool implicitBuffer = true);

    /* Prints a single channel <float> Mat's contents to the debug stream in Qt.
     * Convolution Kernels for input must be 32F for openCV, hence no template
     * Templates must be instantiated in the header so it is visible to the
     * translation unit that uses it (or separate header & included)*/
    template <typename T> void printKernelMatToDebug(const Mat &mat)
    {
        if(mat.empty())
        {
            qDebug("cv::mat is empty - cannot print to debug");
        }

        QString output;

        for(int i = 0; i < mat.cols * mat.rows; i++)
        {
            output.append(QString::number(mat.at<T>(i)) + " ");
            if((i + 1) % mat.cols == 0)
                output.append('\n');
        }
        qDebug(output.toLatin1());
    }
}

#endif // BUFFERWRAPPERSQCV_H
