/***********************************************************************
* FILENAME :    imagesavepngmenu.cpp
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
*       image in the PNG file format.
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
* 0.1           07/21/2019      Matthew R. Miller       Initial Rev
*
************************************************************************/
#include "imagesavepngmenu.h"
#include "ui_imagesavepngmenu.h"
#include "mousewheeleatereventfilter.h"
#include <opencv2/imgcodecs.hpp>

//constructor installs event filter to disable scroll wheel, sets initial states, and connects signals / slots
ImageSavePngMenu::ImageSavePngMenu(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ImageSavePngMenu)
{
    ui->setupUi(this);

    //install event filter
    MouseWheelEaterEventFilter *filter = new MouseWheelEaterEventFilter(this);
    ui->horizontalSlider_Compression->installEventFilter(filter);
    ui->spinBox_Compression->installEventFilter(filter);

    //for documentation, see: https://docs.opencv.org/3.1.0/d4/da8/group__imgcodecs.html
    ui->comboBox_Strategy->addItem("Default", cv::IMWRITE_PNG_STRATEGY_DEFAULT);
    ui->comboBox_Strategy->addItem("Filtered", cv::IMWRITE_PNG_STRATEGY_FILTERED);
    ui->comboBox_Strategy->addItem("Huffman", cv::IMWRITE_PNG_STRATEGY_HUFFMAN_ONLY);
    ui->comboBox_Strategy->addItem("Run-Length", cv::IMWRITE_PNG_STRATEGY_FILTERED);
    ui->comboBox_Strategy->addItem("Fixed", cv::IMWRITE_PNG_STRATEGY_FIXED);

    //connect signals slots
    connect(ui->horizontalSlider_Compression, SIGNAL(valueChanged(int)), ui->spinBox_Compression, SLOT(setValue(int)));
    connect(ui->spinBox_Compression, SIGNAL(valueChanged(int)), ui->horizontalSlider_Compression, SLOT(setValue(int)));
}

//default destructor
ImageSavePngMenu::~ImageSavePngMenu()
{
    delete ui;
}

//returns the compression level - opencv between 0 - 9
int ImageSavePngMenu::getCompression()
{
    return ui->spinBox_Compression->value();
}

//returns the enumerated value for openCV for the compression strategy to be used
int ImageSavePngMenu::getStrategy()
{
    return ui->comboBox_Strategy->currentData().toInt();
}

//returns 1 if binary level enabled, 0 if not
int ImageSavePngMenu::getBinaryLevel()
{
    return ui->checkBox_BinaryLevel->isChecked();
}
