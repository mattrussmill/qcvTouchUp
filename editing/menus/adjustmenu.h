/***********************************************************************
* FILENAME :    adjustmenu.h
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
*       This menu is used for adjusting the color, brightness, contrast,
*       pixel depth, hue, saturation, intensity including high and low
*       adjustments to gamma correction. It collects all necessary slider
*       values in a QVector and copies the object through the signal/signal
*       mechanism to provide the necessary parameters for image processing.
*       The slider values are primed here as necessary for the worker thread.
*
* NOTES :
*       This class is directly tied to adjustmenu.ui and has functionality
*       tied to the ImageWorker class which uses the values selected here
*       as a basis to control calculations on the image.
*
*
* AUTHOR :  Matthew R. Miller       START DATE :    January 02/15/2018
*
* CHANGES : N/A - N/A
*
* VERSION       DATE            WHO                     DETAIL
* 0.1           04/18/2018      Matthew R. Miller       Initial Rev
*
************************************************************************/

#ifndef ADJUSTMENU_H
#define ADJUSTMENU_H

#include <QScrollArea>
#include <QThread>
#include "../../app_filters/signalsuppressor.h"
#include <opencv2/core.hpp>
class QString;
class QMutex;
class AdjustWorker;
class QByteArray;

namespace Ui {
class AdjustMenu;
}

class AdjustMenu : public QScrollArea
{
    Q_OBJECT

public:
    explicit AdjustMenu(QMutex *mutex = nullptr, QWidget *parent = 0);
    ~AdjustMenu();
    enum ParameterIndex
    {
        Brightness  = 0,
        Contrast    = 1,
        Depth       = 2,
        Hue         = 3,
        Saturation  = 4,
        Intensity   = 5,
        Gamma       = 6,
        Highlight   = 7,
        Shadows     = 8,
        Color       = 9
    };

public slots:
    void initializeSliders();
    void receiveImageAddresses(const cv::Mat *masterImage, cv::Mat *previewImage);
    void setMenuTracking(bool enable);
    void setVisible(bool visible) override;
    void showEvent(QShowEvent *event) override;

signals:
    void updateDisplayedImage();
    void distributeImageBufferAddresses(const cv::Mat*,cv::Mat*);
    void updateStatus(QString);

protected:
    const cv::Mat *masterImage_m;
    cv::Mat *previewImage_m;
    QMutex *workerMutex_m;
    QThread worker_m;
    AdjustWorker *adjustWorker_m;

protected slots:
    void manageWorker(bool life);

private:
    Ui::AdjustMenu *ui;
    float sliderValues_m[10];
    SignalSuppressor workSignalSuppressor;

private slots:
    void changeContrastValue(int value);
    void changeBrightnessValue(int value);
    void changeDepthValue(int value);
    void changeHueValue(int value);
    void changeSaturationValue(int value);
    void changeIntensityValue(int value);
    void changeToColorImage();
    void changeToGrayscaleImage();
    void changeGammaValue(int value);
    void changeHighlightsValue(int value);
    void changeShadowsValue(int value);
};

#endif // ADJUSTMENU_H
