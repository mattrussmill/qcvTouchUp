/***********************************************************************
* FILENAME :    temperaturemenu.h
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
*       This menu is used for adjusting the perceived temperature of the image's
*       light source. The menu provides preset options as well as a fine adjustment
*       slider.
*
* NOTES :
*
* AUTHOR :  Matthew R. Miller       START DATE :    April 06/27/2018
*
* CHANGES : N/A - N/A
*
* VERSION       DATE            WHO                     DETAIL
* 0.1           07/02/2018      Matthew R. Miller       Initial Rev
*
************************************************************************/
#ifndef TEMPERATUREMENU_H
#define TEMPERATUREMENU_H

#include <QScrollArea>
#include <QThread>
#include <opencv2/core.hpp>
#include "signalsuppressor.h"
class QString;
class QMutex;
class TemperatureWorker;
class QByteArray;
class QButtonGroup;

namespace Ui {
class TemperatureMenu;
}

class TemperatureMenu : public QScrollArea
{
    Q_OBJECT

public:
    explicit TemperatureMenu(QMutex *mutex, QWidget *parent = 0);
    ~TemperatureMenu();

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
    TemperatureWorker *temperatureWorker_m;

protected slots:
    void manageWorker(bool life);

private:
    Ui::TemperatureMenu *ui;
    QButtonGroup *buttonGroup_m;
    SignalSuppressor workSignalSuppressor;

private slots:
    void moveSliderToButton(bool adjustSlider);
    void sendTemperatureValue(int value);
    void deselectRadioButtonFromSlider();
};

#endif // TEMPERATUREMENU_H
