/***********************************************************************
* FILENAME :    imagesavejpegmenu.cpp
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
*       This widget is used to collect user input parameters for saving an
*       image in the JPEG file format.
*
* NOTES :
*       The values captured are associated with cv::imwrite(), an OpenCV
*       function.
*
*
* AUTHOR :  Matthew R. Miller       START DATE :    March 03/04/2019
*
* CHANGES : N/A - N/A
*
* VERSION       DATE            WHO                     DETAIL
* 0.1           07/07/2019      Matthew R. Miller       Initial Rev
* 0.2           11/05/2019      Matthew R. Miller       Remove chroma/luma quality
*
************************************************************************/
#include "imagesavejpegmenu.h"
#include "ui_imagesavejpegmenu.h"
#include "mousewheeleatereventfilter.h"

//constructor installs event filter to disable scroll wheel, sets initial states, and connects signals / slots
ImageSaveJpegMenu::ImageSaveJpegMenu(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ImageSaveJpegMenu)
{
    ui->setupUi(this);

    //install event filter
    MouseWheelEaterEventFilter *filter = new MouseWheelEaterEventFilter(this);
    ui->horizontalSlider_Quality->installEventFilter(filter);
    ui->horizontalSlider_RestartInterval->installEventFilter(filter);
    ui->spinBox_Quality->installEventFilter(filter);
    ui->spinBox_RestartInterval->installEventFilter(filter);

    //connect signals slots
    connect(ui->horizontalSlider_Quality, SIGNAL(valueChanged(int)), ui->spinBox_Quality, SLOT(setValue(int)));
    connect(ui->spinBox_Quality, SIGNAL(valueChanged(int)), ui->horizontalSlider_Quality, SLOT(setValue(int)));
    connect(ui->horizontalSlider_RestartInterval, SIGNAL(valueChanged(int)), ui->spinBox_RestartInterval, SLOT(setValue(int)));
    connect(ui->spinBox_RestartInterval, SIGNAL(valueChanged(int)), ui->horizontalSlider_RestartInterval, SLOT(setValue(int)));
}

//default destructor
ImageSaveJpegMenu::~ImageSaveJpegMenu()
{
    delete ui;
}

//returns the overall quality value for the JPEG image - opencv between 0 - 100
int ImageSaveJpegMenu::getQuality()
{
    return ui->spinBox_Quality->value();
}

//returns the restart interval for the JPEG image - opencv between 0 - 65535, 0 = none
int ImageSaveJpegMenu::getRestartInterval()
{
    return ui->spinBox_RestartInterval->value();
}

//returns 1 if progressive scan is enabled, 0 if not used
int ImageSaveJpegMenu::getProgressiveScan()
{
    return static_cast<int>(ui->checkBox_ProgressiveScan->isChecked());
}

//returns 1 if optomized baseline is enabled, 0 if not used
int ImageSaveJpegMenu::getBaselineOptimized()
{
    return static_cast<int>(ui->checkBox_Optomized->isChecked());
}
