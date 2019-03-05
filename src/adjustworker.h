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
*
************************************************************************/

#ifndef ADJUSTWORKER_H
#define ADJUSTWORKER_H

#include <QObject>
#include <QByteArray>
#include <opencv2/core.hpp>
class QMutex;
class QString;
class SignalSuppressor;

class AdjustWorker : public QObject
{
    Q_OBJECT
public:
    explicit AdjustWorker(const cv::Mat *masterImage = nullptr, cv::Mat *previewImage = nullptr, QMutex *mutex = nullptr, QObject *parent = nullptr);
    ~AdjustWorker();

signals:
    void updateDisplayedImage();
    void updateStatus(QString);

public slots:
    void receiveSuppressedSignal(SignalSuppressor *dataContainer);
    void receiveImageAddresses(const cv::Mat *masterImage, cv::Mat *previewImage);

private:
    void performImageAdjustments(float *parameter);
    QByteArray data;
    QMutex *mutex_m;
    const cv::Mat *masterImage_m;
    cv::Mat *previewImage_m;
    cv::UMat implicitOclImage_m;
    cv::Mat lookUpTable_m;
    std::vector <cv::UMat> splitChannelsTmp_m;
};

#endif // ADJUSTWORKER_H
