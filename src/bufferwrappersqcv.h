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

#include <QImage>
#include <QPixmap>
#include <QString>
#include <opencv2/core/core.hpp>

namespace qcv
{
    using namespace cv;
    QString getMatType(const Mat &image);
    QImage cvMatToQImage(const Mat &image, const bool implicitBuffer = true);
    Mat qImageToCvMat(const QImage &image, const bool implicitBuffer = true);
    void printMatToDebug(const Mat &mat);
}

#endif // BUFFERWRAPPERSQCV_H
