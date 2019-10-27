/***********************************************************************
* FILENAME :    transformworker.h
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
*       This is the worker thread object tied to the transformmenu.cpp object.
*       The worker performs the operations for performing matrix transformations.
*
* NOTES :
*       This worker thread uses OpenCV OpenCL accelerated function calls implicitly
*       when OpenCL hardware is available through OpenCV's UMat object calls. In the
*       constructor there is an OpenCL initialization step where the OpenCL commands
*       are given before the functionality is actually used through calling the
*       appropriate method prematurely. See Issue #41 for more detail.
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
#ifndef TRANSFORMWORKER_H
#define TRANSFORMWORKER_H

#include <QObject>
#include <QRect>
#include <opencv2/core.hpp>
class QMutex;
class QString;
class SignalSuppressor;

class TransformWorker : public QObject
{
    Q_OBJECT
public:
    explicit TransformWorker(const cv::Mat *masterImage = nullptr, cv::Mat *previewImage = nullptr, QMutex *mutex = nullptr, QObject *parent = nullptr);
    ~TransformWorker();

signals:
    void updateDisplayedImage();
    void updateStatus(QString);
    void handleExceptionMessage(QString);

public slots:
    void receiveRotateSuppressedSignal(SignalSuppressor *dataContainer);
    void receiveScaleSuppressedSignal(SignalSuppressor *dataContainer);
    void receiveImageAddresses(const cv::Mat *masterImage, cv::Mat *previewImage);
    void doCropComputation(QRect);
    void setAutoCropForRotate(bool);
    void doRotateComputation(int);
    void doScaleComputation(QRect);

private:
    QMutex *mutex_m;
    bool autoCropforRotate_m;
    const cv::Mat *masterImage_m;
    cv::Mat *previewImage_m;
    cv::UMat implicitOclImage_m;
    cv::UMat previewImplicitOclImage_m;

};

#endif // TRANSFORMWORKER_H
