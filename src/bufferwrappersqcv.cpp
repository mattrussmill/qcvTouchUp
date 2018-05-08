/***********************************************************************
* FILENAME :    bufferwrappersqcv.c
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
#include "bufferwrappersqcv.h"
#include <QImage>
#include <QPixmap>
#include <QString>
#include <QDebug>

#include <opencv2/core/core.hpp>

namespace qcv
{
    using namespace cv;

    /* This function returns the QString equivalent of the OpenCV Mat::type().
     * If the type is undefined then the function returns an empty QString. */
    QString getMatType(const Mat &image)
    {
        QString type;

        switch (image.depth())
        {
        case 0:
            type.append("8U");
            break;
        case 1:
            type.append("8S");
            break;
        case 2:
            type.append("16U");
            break;
        case 3:
            type.append("16S");
            break;
        case 4:
            type.append("32S");
            break;
        case 5:
            type.append("32F");
            break;
        case 6:
            type.append("64F");
            break;
        default:
            return type;
        }

        return type.append(QString::number(image.channels()));
    }

    /* This function creates a QImage container for an OpenCV Mat containing either an RGB, 3-channel color
     * image or a 1-channel grayscale image. The imagebuffer between these two containers are implicitly
     * shared if implicitBuffer is TRUE to avoid large copying operations. During destruction, if implicitly
     * shared, detach the buffer from the image before the Mat is deleted. If it fails an empty QImage is
     * returned. */
    QImage cvMatToQImage(const Mat &image, const bool implicitBuffer)
    {
        switch(image.type())
        {

        case CV_8UC3: //RGB Format Assumed
        {
            QImage newImage(image.data, image.cols, image.rows, static_cast<int>(image.step),
                            QImage::Format_RGB888);
            return (implicitBuffer ? newImage : newImage.copy(0, 0, newImage.width(), newImage.height()));
        }
        case CV_8UC1: //Grayscale Format Assumed
        {
            QImage newImage(image.data, image.cols, image.rows, static_cast<int>(image.step),
                            QImage::Format_Grayscale8);
            return (implicitBuffer ? newImage : newImage.copy(0, 0, newImage.width(), newImage.height()));
        }
        default:
        {
            qWarning() << "qcv::MatToQImage() - Mat type " + getMatType(image) + " not supported!";
            break;
        }

        }
        return QImage();
    }

    /* This function creates an OpenCV Mat container for a QImage containing either an RGB, 3-channel color
     * image or a 1-channel grayscale image. The imagebuffer between these two containers are implicitly
     * shared if implicitBuffer is TRUE to avoid large copying operations. During destruction, if implicitly
     * shared, detach the buffer from the image before the Mat is deleted. If it fails an empty Mat is
     * returned. Mats must be converted to BGR before saving or displaying with OpenCV functions/methods. */
    Mat qImageToCvMat(const QImage &image, const bool implicitBuffer)
    {
        switch(image.format())
        {

        case QImage::Format_RGB888:
        {
            Mat newImage(image.height(), image.width(), CV_8UC3, const_cast<uchar*>(image.bits()),
                         static_cast<size_t>(image.bytesPerLine()));
            return (implicitBuffer ? newImage : newImage.clone());
        }
        case QImage::Format_Grayscale8:
        {
            Mat newImage(image.height(), image.width(), CV_8UC1, const_cast<uchar*>(image.bits()),
                         static_cast<size_t>(image.bytesPerLine()));
            return (implicitBuffer ? newImage : newImage.clone());
        }
        default:
        {
            qWarning() << "qcv::MatToQImage() - QImage format enum " + QString::number(image.format()) + " not supported!";
            break;
        }

        }
        return Mat();
    }

}
