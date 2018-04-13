/***********************************************************************
* FILENAME :    bufferwrappersqcv.h
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
}

#endif // BUFFERWRAPPERSQCV_H
