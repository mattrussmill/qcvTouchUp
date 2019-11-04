/***********************************************************************
* FILENAME :    mainwindow.h
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
*       This is the main window of the application. It is used as the parent
*       object for the application. All object placement and communication
*       can be traced back to this object.
*
* AUTHOR :  Matthew R. Miller       START DATE :    March 03/04/2019
*
* CHANGES : N/A - N/A
*
* VERSION       DATE            WHO                     DETAIL
* 0.1           11/03/2019      Matthew R. Miller       Initial Rev
*
************************************************************************/
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <opencv2/core.hpp>
#include <QMutex>
#include <QDir>
class QImage;
class QString;
class AdjustMenu;
class FilterMenu;
class TemperatureMenu;
class TransformMenu;
//class ColorSliceMenu;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

signals:
    void setImage(const QImage*);
    void distributeImageBufferAddresses(const cv::Mat*, cv::Mat*);
    void setDefaultTracking(bool);

protected:
    QMutex mutex_m;

private slots:
    void imageOpenOperationFailed();
    void updateImageInformation(const QImage *image);
    void getImagePath();
    void cancelPreview();
    void applyPreviewToMaster();
    void displayPreview();
    void saveImageAs();
    bool loadImageIntoMemory(QString imagePath);
    void generateAboutDialog();

private:
    void clearImageBuffers();
    Ui::MainWindow *ui;
    QDir userImagePath_m;
    cv::Mat masterRGBImage_m;
    cv::Mat previewRGBImage_m;
    QImage imageWrapper_m;

    //menus
    AdjustMenu *adjustMenu_m;
    FilterMenu *filterMenu_m;
    TemperatureMenu *temperatureMenu_m;
    TransformMenu *transformMenu_m;
    //ColorSliceMenu *colorSliceMenu_m;
};

#endif // MAINWINDOW_H
