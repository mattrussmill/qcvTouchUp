/***********************************************************************
* FILENAME :    filtermenu.cpp
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
*       This menu is used for filtering the image through 2D kernels by
*       performing high and low pass filtering. This filtering includes
*       sharpening, smoothing/blurring, and edge detection. It collects
*       all necessary slider values in a QVector and copies the object
*       through the signal/signal mechanism to provide the necessary
*       parameters for image processing.
*
*
* NOTES :
*       This class is directly tied to filtermenu.ui and has functionality
*       tied to the ImageWorker class which uses the values selected here
*       as a basis to control calculations on the image.
*
*
* AUTHOR :  Matthew R. Miller       START DATE :    April 04/16/2018
*
* CHANGES : N/A - N/A
*
* VERSION       DATE            WHO                     DETAIL
* 0.1           06/23/2018      Matthew R. Miller       Initial Rev
*
************************************************************************/

#include "mousewheeleatereventfilter.h"
#include "filtermenu.h"
#include "ui_filtermenu.h"
#include <QScrollArea>
#include <QVector>
#include <QPixmap>
#include <QButtonGroup>

//Constructor installs the MouseWheelEaterFilter for all sliders, resizes the parameter
//QVector appropriately, sets up the ComboBox(es) and establishes all signals/slots necessary.
FilterMenu::FilterMenu(QWidget *parent) :
    QScrollArea(parent),
    ui(new Ui::FilterMenu)
{
    ui->setupUi(this);
    MouseWheelEaterEventFilter *wheelFilter = new MouseWheelEaterEventFilter(this);

    //fix radio buttons to work in separate group boxes (for asthetics)
    QButtonGroup *radioGroup = new QButtonGroup(this);
    radioGroup->addButton(ui->radioButton_SmoothEnable);
    radioGroup->addButton(ui->radioButton_SharpenEnable);
    radioGroup->addButton(ui->radioButton_EdgeEnable);

    //setup smooth menu options
    ui->comboBox_Smooth->addItem("Average");    //comboBox index 0 (default)
    ui->comboBox_Smooth->addItem("Gaussian");   //comboBox index 1
    ui->comboBox_Smooth->addItem("Median");     //comboBox index 2
    ui->comboBox_Smooth->installEventFilter(wheelFilter);
    ui->horizontalSlider_SmoothWeight->installEventFilter(wheelFilter);
    connect(ui->radioButton_SmoothEnable, SIGNAL(released()), this, SLOT(collectBlurParameters()));
    connect(ui->radioButton_SmoothEnable, SIGNAL(released()), this, SLOT(changeSampleImage()));
    connect(ui->comboBox_Smooth, SIGNAL(currentIndexChanged(int)), this, SLOT(collectBlurParameters()));
    connect(ui->horizontalSlider_SmoothWeight, SIGNAL(valueChanged(int)), this, SLOT(collectBlurParameters()));

    //setup sharpen menu options
    ui->comboBox_Sharpen->addItem("Unsharpen"); //comboBox index 0 (default)
    ui->comboBox_Sharpen->addItem("Laplacian"); //comboBox index 1
    ui->comboBox_Sharpen->installEventFilter(wheelFilter);
    ui->horizontalSlider_SharpenWeight->installEventFilter(wheelFilter);
    connect(ui->radioButton_SharpenEnable, SIGNAL(released()), this, SLOT(collectSharpenParameters()));
    connect(ui->radioButton_SharpenEnable, SIGNAL(released()), this, SLOT(changeSampleImage()));
    connect(ui->comboBox_Sharpen, SIGNAL(currentIndexChanged(int)), this, SLOT(adjustSharpenSliderRange(int)));
    connect(ui->comboBox_Sharpen, SIGNAL(currentIndexChanged(int)), this, SLOT(collectSharpenParameters()));
    connect(ui->horizontalSlider_SharpenWeight, SIGNAL(valueChanged(int)), this, SLOT(collectSharpenParameters()));

    //setup edge detect menu options
    ui->comboBox_Edge->addItem("Canny");        //comboBox index 0 (default)
    ui->comboBox_Edge->addItem("Laplacian");    //comboBox index 1 - must match above
    ui->comboBox_Edge->addItem("Sobel");        //comboBox index 2
    ui->comboBox_Edge->installEventFilter(wheelFilter);
    ui->horizontalSlider_EdgeWeight->installEventFilter(wheelFilter);
    connect(ui->radioButton_EdgeEnable, SIGNAL(released()), this, SLOT(collectEdgeDetectParameters()));
    connect(ui->radioButton_EdgeEnable, SIGNAL(released()), this, SLOT(changeSampleImage()));
    connect(ui->comboBox_Edge, SIGNAL(currentIndexChanged(int)), this, SLOT(adjustEdgeSliderRange(int)));
    connect(ui->comboBox_Edge, SIGNAL(currentIndexChanged(int)), this, SLOT(collectEdgeDetectParameters()));
    connect(ui->horizontalSlider_EdgeWeight, SIGNAL(valueChanged(int)), this, SLOT(collectEdgeDetectParameters()));

    menuValues.resize(2);
    initializeSliders();
}

// destructor
FilterMenu::~FilterMenu()
{
    delete ui;
}

// Function initializes the necessary widget values to their starting values.
void FilterMenu::initializeSliders()
{
    this->blockSignals(true);

    ui->horizontalSlider_SmoothWeight->setValue(ui->horizontalSlider_SmoothWeight->minimum());
    ui->horizontalSlider_SharpenWeight->setValue(ui->horizontalSlider_SharpenWeight->minimum());
    ui->horizontalSlider_EdgeWeight->setValue(ui->horizontalSlider_EdgeWeight->minimum());

    this->blockSignals(false);
}

//Changes the slider range for the SharpenSlider based on the needs of the filter selected from the combo box.
void FilterMenu::adjustSharpenSliderRange(int value)
{
    //if not default filter (unsharpen), switch to maximum value of 50 for no gap in slider values
    if(value == FilterLaplacian)
    {
        ui->horizontalSlider_SharpenWeight->setValue(ui->horizontalSlider_SharpenWeight->value() / 2);
        ui->horizontalSlider_SharpenWeight->setMaximum(50);
        ui->horizontalSlider_SharpenWeight->setPageStep(2);
    }
    else
    {
        ui->horizontalSlider_SharpenWeight->setMaximum(100);
        ui->horizontalSlider_SharpenWeight->setValue(ui->horizontalSlider_SharpenWeight->value() * 2);
        ui->horizontalSlider_SharpenWeight->setPageStep(3);
    }
}

//Changes the slider range for the EdgeSlider based on the needs of the filter selected from the combo box.
void FilterMenu::adjustEdgeSliderRange(int value)
{
    //if canny filter, switch to minimum value to 1 (1*2+1=3) in passing slot for no gap in slider values
    if(value == FilterCanny)
    {
        ui->horizontalSlider_EdgeWeight->setValue(1);
        ui->horizontalSlider_EdgeWeight->setMinimum(1);
    }
    else
    {
        ui->horizontalSlider_EdgeWeight->setMinimum(0);
    }
}

//Populates the menuValues parameter and passes it to a worker slot for the Smooth operation.
void FilterMenu::collectBlurParameters()
{
    //if filter not enabled, do nothing
    if(!ui->radioButton_SmoothEnable->isChecked()) return;

    menuValues[KernelType] = ui->comboBox_Smooth->currentIndex();
    menuValues[KernelWeight] = ui->horizontalSlider_SmoothWeight->value();

    emit performImageBlur(menuValues);
}

//Populates the menuValues parameter and passes it to a worker slot for the Sharpen operation.
void FilterMenu::collectSharpenParameters()
{
    //if filter not enabled, do nothing
    if(!ui->radioButton_SharpenEnable->isChecked()) return;

    menuValues[KernelType] = ui->comboBox_Sharpen->currentIndex();
    menuValues[KernelWeight] = ui->horizontalSlider_SharpenWeight->value();

    emit performImageSharpen(menuValues);
}

//Populates the menuValues parameter and passes it to a worker slot for the Edge Detect operation.
void FilterMenu::collectEdgeDetectParameters()
{
    //if filter not enabled, do nothing
    if(!ui->radioButton_EdgeEnable->isChecked()) return;

    //sends values of 1/3/5/7 for opencv functions. Slider ranges from 0 to 3
    menuValues[KernelType] = ui->comboBox_Edge->currentIndex();
    menuValues[KernelWeight] = ui->horizontalSlider_EdgeWeight->value() * 2 + 1;

    emit performImageEdgeDetect(menuValues);
}

//Sets the sample image based on the menu item selected.
void FilterMenu::changeSampleImage()
{
    if(ui->radioButton_SmoothEnable->isChecked())
        ui->label_SampleImage->setPixmap(QPixmap::fromImage(QImage(":/img/icons/filterMenu/blur.png")));
    else if(ui->radioButton_SharpenEnable->isChecked())
        ui->label_SampleImage->setPixmap(QPixmap::fromImage(QImage(":/img/icons/filterMenu/sharp.png")));
    else if(ui->radioButton_EdgeEnable->isChecked())
        ui->label_SampleImage->setPixmap(QPixmap::fromImage(QImage(":/img/icons/filterMenu/edge.png")));
}

//overloads setVisible to signal the worker thread to cancel any adjustments that weren't applied when minimized
void FilterMenu::setVisible(bool visible)
{
    if(this->isVisible() && !visible)
        emit cancelAdjustments();
    QWidget::setVisible(visible);
}

//Sets sliders to initial positions and signals the worker to apply the changes to the master buffer.
void FilterMenu::on_pushButton_Apply_released()
{
    initializeSliders();
    emit applyAdjustments();
}

//Sets sliders to initial positions and signals to the worker to display the starting buffer.
void FilterMenu::on_pushButton_Cancel_released()
{
    initializeSliders();
    emit cancelAdjustments();
}
