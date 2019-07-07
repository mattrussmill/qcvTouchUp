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
*
************************************************************************/
#include "imagesavejpegmenu.h"
#include "ui_imagesavejpegmenu.h"
#include "mousewheeleatereventfilter.h"

ImageSaveJpegMenu::ImageSaveJpegMenu(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ImageSaveJpegMenu)
{
    ui->setupUi(this);

    //install event filter
    MouseWheelEaterEventFilter *filter = new MouseWheelEaterEventFilter(this);
    ui->horizontalSlider_Quality->installEventFilter(filter);
    ui->horizontalSlider_RestartInterval->installEventFilter(filter);
    ui->horizontalSlider_Chroma->installEventFilter(filter);
    ui->horizontalSlider_Luma->installEventFilter(filter);
    ui->spinBox_Quality->installEventFilter(filter);
    ui->spinBox_RestartInterval->installEventFilter(filter);
    ui->spinBox_Chroma->installEventFilter(filter);
    ui->spinBox_Luma->installEventFilter(filter);

    //initial states
    ui->horizontalSlider_Chroma->setEnabled(false);
    ui->spinBox_Chroma->setEnabled(false);
    ui->horizontalSlider_Luma->setEnabled(false);
    ui->spinBox_Luma->setEnabled(false);

    //connect signals slots
    connect(ui->horizontalSlider_Quality, SIGNAL(valueChanged(int)), ui->spinBox_Quality, SLOT(setValue(int)));
    connect(ui->spinBox_Quality, SIGNAL(valueChanged(int)), ui->horizontalSlider_Quality, SLOT(setValue(int)));
    connect(ui->horizontalSlider_RestartInterval, SIGNAL(valueChanged(int)), ui->spinBox_RestartInterval, SLOT(setValue(int)));
    connect(ui->spinBox_RestartInterval, SIGNAL(valueChanged(int)), ui->horizontalSlider_RestartInterval, SLOT(setValue(int)));
    connect(ui->horizontalSlider_Chroma, SIGNAL(valueChanged(int)), ui->spinBox_Chroma, SLOT(setValue(int)));
    connect(ui->spinBox_Chroma, SIGNAL(valueChanged(int)), ui->horizontalSlider_Chroma, SLOT(setValue(int)));
    connect(ui->horizontalSlider_Luma, SIGNAL(valueChanged(int)), ui->spinBox_Luma, SLOT(setValue(int)));
    connect(ui->spinBox_Luma, SIGNAL(valueChanged(int)), ui->horizontalSlider_Luma, SLOT(setValue(int)));

    connect(ui->checkBox_EnableChroma, SIGNAL(toggled(bool)), ui->horizontalSlider_Chroma, SLOT(setEnabled(bool)));
    connect(ui->checkBox_EnableChroma, SIGNAL(toggled(bool)), ui->spinBox_Chroma, SLOT(setEnabled(bool)));
    connect(ui->checkBox_EnableLuma, SIGNAL(toggled(bool)), ui->horizontalSlider_Luma, SLOT(setEnabled(bool)));
    connect(ui->checkBox_EnableLuma, SIGNAL(toggled(bool)), ui->spinBox_Luma, SLOT(setEnabled(bool)));
}

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

//returns the quality value for the Luma channel individually - opencv between 0 - 100, 0 = done use
int ImageSaveJpegMenu::getLumaQuality()
{
    if(ui->checkBox_EnableLuma->isChecked())
        return ui->spinBox_Luma->value();
    return 0;
}

//returns the quality value for the Chroma channel individually - opencv between 0 - 100, 0 = done use
int ImageSaveJpegMenu::getChromaQuality()
{
    if(ui->checkBox_EnableChroma->isChecked())
        return ui->spinBox_Chroma->value();
    return 0;
}

//returns 1 if progressive scan is enabled, 0 if not used
int ImageSaveJpegMenu::getProgressiveScan()
{
    return ui->checkBox_ProgressiveScan->isChecked();
}

//returns 1 if optomized baseline is enabled, 0 if not used
int ImageSaveJpegMenu::getBaselineOptimized()
{
    return ui->checkBox_Optomized->isChecked();
}
