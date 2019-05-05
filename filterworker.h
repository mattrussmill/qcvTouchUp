/***********************************************************************
* FILENAME :    filterworker.h
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
#ifndef FILTERWORKER_H
#define FILTERWORKER_H

#include <QObject>
#include <QByteArray>
#include <opencv2/core.hpp>
class QMutex;
class QString;
class SignalSuppressor;

class FilterWorker : public QObject
{
    Q_OBJECT
public:
    explicit FilterWorker(const cv::Mat *masterImage = nullptr, cv::Mat *previewImage = nullptr, QMutex *mutex = nullptr, QObject *parent = nullptr);
    ~FilterWorker();

signals:
    void updateDisplayedImage();
    void updateStatus(QString);

public slots:
    void receiveSuppressedSignal(SignalSuppressor *dataContainer);
    void receiveImageAddresses(const cv::Mat *masterImage, cv::Mat *previewImage);

private:
    void performImageFiltering(int *parameter);
    int kernelSize(QSize image, int weightPercent);
    cv::Mat makeLaplacianKernel(int size);
    QByteArray data_m;
    QMutex *mutex_m;
    const cv::Mat *masterImage_m;
    cv::Mat *previewImage_m;
    cv::Mat tmpImage_m;
};

#endif // FILTERWORKER_H
