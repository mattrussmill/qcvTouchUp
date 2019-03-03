/***********************************************************************
* FILENAME :    adjustmenu.cpp
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
#include "adjustmenu.h"
#include "ui_adjustmenu.h"
#include "adjustworker.h"
#include "mousewheeleatereventfilter.h"
#include <cmath>
#include <QMutex>
#include <QByteArray>
#include <QDebug>

//Constructor installs the MouseWheelEaterFilter for all sliders, resizes the parameter
//QVector appropriately, and establishes all signals/slots necessary for the ui.
AdjustMenu::AdjustMenu(QMutex *mutex, QWidget *parent) :
    QScrollArea(parent),
    ui(new Ui::AdjustMenu)
{
    ui->setupUi(this);
    masterImage_m = nullptr;
    previewImage_m = nullptr;
    workerMutex_m = mutex;
    adjustWorker_m = nullptr;

    MouseWheelEaterEventFilter *wheelFilter = new MouseWheelEaterEventFilter(this);
    ui->horizontalSlider_Brightness->installEventFilter(wheelFilter);
    ui->horizontalSlider_Contrast->installEventFilter(wheelFilter);
    ui->horizontalSlider_Depth->installEventFilter(wheelFilter);
    ui->horizontalSlider_Hue->installEventFilter(wheelFilter);
    ui->horizontalSlider_Saturation->installEventFilter(wheelFilter);
    ui->horizontalSlider_Intensity->installEventFilter(wheelFilter);
    ui->horizontalSlider_Gamma->installEventFilter(wheelFilter);
    ui->horizontalSlider_Highlight->installEventFilter(wheelFilter);
    ui->horizontalSlider_Shadows->installEventFilter(wheelFilter);

    //adjust menu signals / slots
    connect(ui->horizontalSlider_Contrast, SIGNAL(valueChanged(int)), this, SLOT(changeContrastValue(int)));
    connect(ui->horizontalSlider_Brightness, SIGNAL(valueChanged(int)), this, SLOT(changeBrightnessValue(int)));
    connect(ui->horizontalSlider_Depth, SIGNAL(valueChanged(int)), this, SLOT(changeDepthValue(int)));
    connect(ui->horizontalSlider_Hue, SIGNAL(valueChanged(int)), this, SLOT(changeHueValue(int)));
    connect(ui->horizontalSlider_Saturation, SIGNAL(valueChanged(int)), this, SLOT(changeSaturationValue(int)));
    connect(ui->horizontalSlider_Intensity, SIGNAL(valueChanged(int)), this, SLOT(changeIntensityValue(int)));
    connect(ui->horizontalSlider_Gamma, SIGNAL(valueChanged(int)), this, SLOT(changeGammaValue(int)));
    connect(ui->horizontalSlider_Highlight, SIGNAL(valueChanged(int)), this, SLOT(changeHighlightsValue(int)));
    connect(ui->horizontalSlider_Shadows, SIGNAL(valueChanged(int)), this, SLOT(changeShadowsValue(int)));
    connect(ui->radioButton_Color, SIGNAL(released()), this, SLOT(changeToColorImage()));
    connect(ui->radioButton_Grayscale, SIGNAL(released()), this, SLOT(changeToGrayscaleImage()));

    initializeSliders();
}

// destructor
AdjustMenu::~AdjustMenu()
{
    //end worker thread once event loop finishes
    if(adjustWorker_m)
    {
        worker_m.terminate();
        worker_m.wait();
        delete adjustWorker_m; //safe? Different way to handle this?
        adjustWorker_m = nullptr;
    }

    delete ui;
}

void AdjustMenu::receiveImageAddresses(const cv::Mat *masterImage, cv::Mat *previewImage)
{
    masterImage_m = masterImage;
    previewImage_m = previewImage;
    qDebug() << "Adjust Menu Images:" << masterImage_m << previewImage_m;
    emit distributeImageBufferAddresses(masterImage, previewImage);
}

void AdjustMenu::setMenuTracking(bool enable)
{
    ui->horizontalSlider_Brightness->setTracking(enable);
    ui->horizontalSlider_Contrast->setTracking(enable);
    ui->horizontalSlider_Depth->setTracking(enable);
    ui->horizontalSlider_Gamma->setTracking(enable);
    ui->horizontalSlider_Highlight->setTracking(enable);
    ui->horizontalSlider_Hue->setTracking(enable);
    ui->horizontalSlider_Intensity->setTracking(enable);
    ui->horizontalSlider_Saturation->setTracking(enable);
    ui->horizontalSlider_Shadows->setTracking(enable);
}

/* Function initializes the sliders and corresponding shared array (for passing
 * slider values) while signals are blocked from being emitted from the class
 * as to not signal the worker thread to re-perform operations during initialization*/
void AdjustMenu::initializeSliders()
{
    //disable signals from being emitted by object
    ui->horizontalSlider_Brightness->blockSignals(true);
    ui->horizontalSlider_Contrast->blockSignals(true);
    ui->horizontalSlider_Depth->blockSignals(true);
    ui->horizontalSlider_Hue->blockSignals(true);
    ui->horizontalSlider_Saturation->blockSignals(true);
    ui->horizontalSlider_Intensity->blockSignals(true);
    ui->horizontalSlider_Gamma->blockSignals(true);
    ui->horizontalSlider_Highlight->blockSignals(true);
    ui->horizontalSlider_Shadows->blockSignals(true);
    ui->radioButton_Color->blockSignals(true);
    ui->radioButton_Grayscale->blockSignals(true);

    //reset slider positions
    ui->horizontalSlider_Brightness->setValue(0);
    ui->horizontalSlider_Contrast->setValue(100);
    ui->horizontalSlider_Depth->setValue(128);
    ui->horizontalSlider_Hue->setValue(0);
    ui->horizontalSlider_Saturation->setValue(0);
    ui->horizontalSlider_Intensity->setValue(0);
    ui->horizontalSlider_Gamma->setValue(0);
    ui->horizontalSlider_Highlight->setValue(0);
    ui->horizontalSlider_Shadows->setValue(0);
    ui->radioButton_Color->setChecked(true);
    ui->radioButton_Grayscale->setChecked(false);

    //enable signals from being emitted by object
    ui->horizontalSlider_Brightness->blockSignals(false);
    ui->horizontalSlider_Contrast->blockSignals(false);
    ui->horizontalSlider_Depth->blockSignals(false);
    ui->horizontalSlider_Hue->blockSignals(false);
    ui->horizontalSlider_Saturation->blockSignals(false);
    ui->horizontalSlider_Intensity->blockSignals(false);
    ui->horizontalSlider_Gamma->blockSignals(false);
    ui->horizontalSlider_Highlight->blockSignals(false);
    ui->horizontalSlider_Shadows->blockSignals(false);
    ui->radioButton_Color->blockSignals(false);
    ui->radioButton_Grayscale->blockSignals(false);

    //Set initial parameter array for sliders
    sliderValues_m[Brightness] = 0.0; //cast to byte array so can be transferred via QVariant
    sliderValues_m[Contrast] = 1.0;
    sliderValues_m[Depth] = 255.0;
    sliderValues_m[Hue] = 0.0;
    sliderValues_m[Saturation] = 0.0;
    sliderValues_m[Intensity] = 0.0;
    sliderValues_m[Gamma] = 1.0;
    sliderValues_m[Highlight] = 1.0;
    sliderValues_m[Shadows] = 1.0;
    sliderValues_m[Color] = 1.0;
}

/* Slot signals contrast worker when triggered. When working with images represented by 8-bits per channel,
 * normalizing the contrast coefficient between 0.1 and 2.4 is preferred. To do so, the slider ranges
 * from 10 to 190 and shifts the decimal to the left before passing the alpha coefficient to the worker. */
void AdjustMenu::changeContrastValue(int value)
{
    //if > 1, increase range from 1 to 2.4 while keeping 1 the slider center point by using log10
    if(value > 100) value *= log10(value / 10.0);
    sliderValues_m[Contrast] = value / 100.0;
    workSignalSuppressor.receiveNewData(QByteArray(reinterpret_cast<char*>(&sliderValues_m), sizeof(float) * 10));
}

/* Slot adjusts brightness slider when triggered for RGB values. It adjusts the sliders
 * between 127 and -127. Once the slider values are adjusted and the appropriate value stored in the sliderValues
 * parameter array, the signal is emitted to the image worker to perform the adjustments on the image. */
void AdjustMenu::changeBrightnessValue(int value)
{
    sliderValues_m[Brightness] = value;
    workSignalSuppressor.receiveNewData(QByteArray(reinterpret_cast<char*>(&sliderValues_m), sizeof(float) * 10));
}

/* Slot adjusts the number of intensity values per channel allotted in the image. The value is adjusted on
 * an exponential curve to favor values, and have higher resolution, as the number of allotted intensity values
 * approaches 1 for the image. To the user, change is less noticeable the closer it is to 255.*/
void AdjustMenu::changeDepthValue(int value)
{
    sliderValues_m[Depth] = pow(value, value / 255.0); // difference more noticeable to the eye closer to 1
    workSignalSuppressor.receiveNewData(QByteArray(reinterpret_cast<char*>(&sliderValues_m), sizeof(float) * 10));
}

/* Slot adjusts hue slider when triggered for HLS values. It shifts the hue value from the sliders
 * by 180 to -180 degrees. Once the slider values are adjusted and the appropriate value stored in the sliderValues
 * parameter array, the signal is emitted to the image worker to perform the adjustments on the image. */
void AdjustMenu::changeHueValue(int value)
{
    sliderValues_m[Hue] = value;
    workSignalSuppressor.receiveNewData(QByteArray(reinterpret_cast<char*>(&sliderValues_m), sizeof(float) * 10));
}

/* Slot adjusts saturation slider when triggered for HLS values. It adjusts the sliders
 * between 127 and -127. Once the slider values are adjusted and the appropriate value stored in the sliderValues
 * parameter array, the signal is emitted to the image worker to perform the adjustments on the image. */
void AdjustMenu::changeSaturationValue(int value)
{
    sliderValues_m[Saturation] = value;
    workSignalSuppressor.receiveNewData(QByteArray(reinterpret_cast<char*>(&sliderValues_m), sizeof(float) * 10));
}

/* Slot adjusts intensity (or lightness) slider when triggered for HLS values. It adjusts the sliders
 * between 127 and -127. Once the slider values are adjusted and the appropriate value stored in the sliderValues
 * parameter array, the signal is emitted to the image worker to perform the adjustments on the image. */
void AdjustMenu::changeIntensityValue(int value)
{
    sliderValues_m[Intensity] = value;
    workSignalSuppressor.receiveNewData(QByteArray(reinterpret_cast<char*>(&sliderValues_m), sizeof(float) * 10));
}

//Sets radio button to generate a color image
void AdjustMenu::changeToColorImage()
{
    sliderValues_m[Color] = 1.0;
    workSignalSuppressor.receiveNewData(QByteArray(reinterpret_cast<char*>(&sliderValues_m), sizeof(float) * 10));
}

//Sets radio button to generate a grayscale image
void AdjustMenu::changeToGrayscaleImage()
{
    sliderValues_m[Color] = -1.0;
    workSignalSuppressor.receiveNewData(QByteArray(reinterpret_cast<char*>(&sliderValues_m), sizeof(float) * 10));
}

/* Slot adjusts gamma over the whole range of intensities in the image. It adjusts the sliders between -100 and 100.
 * When the slider value is below zero it sets the gamma correction between 1 and approx 0.3. When the value is above
 * zero, it sets the gamma correction between 1 and 3.*/
void AdjustMenu::changeGammaValue(int value)
{
    if (value < 0)
        sliderValues_m[Gamma] = (value / 150.0) + 1;
    else
        sliderValues_m[Gamma] = (value / 50.0) + 1;
    workSignalSuppressor.receiveNewData(QByteArray(reinterpret_cast<char*>(&sliderValues_m), sizeof(float) * 10));
}

/* Slot shifts the gamma adjustment plot vertically between 80 and -80 for values affected between 149 and 255*/
void AdjustMenu::changeHighlightsValue(int value)
{
    sliderValues_m[Highlight] = value; //should use highlight method with equations provided
    workSignalSuppressor.receiveNewData(QByteArray(reinterpret_cast<char*>(&sliderValues_m), sizeof(float) * 10));
}

/* Slot shifts the gamma adjustment plot vertically between 80 and -80 for values affected between 0 and 106*/
void AdjustMenu::changeShadowsValue(int value)
{
    sliderValues_m[Shadows] = value;
    workSignalSuppressor.receiveNewData(QByteArray(reinterpret_cast<char*>(&sliderValues_m), sizeof(float) * 10));
}

//overloads setVisible to signal the worker thread to cancel any adjustments that weren't applied when minimized
void AdjustMenu::setVisible(bool visible) //https://doc.qt.io/qt-5/qwidget.html#visible-prop
{                                           //use showevent override instead
    manageWorker(visible);
    if(!visible)
        initializeSliders();
    QWidget::setVisible(visible);
}

void AdjustMenu::manageWorker(bool life)
{
    if(life)
    {
        if(!adjustWorker_m)
        {
            //If worker is still trying to exit, wait and process other events until its done
            if(worker_m.isFinished())
            {
                QApplication::setOverrideCursor(Qt::WaitCursor);
                qDebug() << "Waiting for thread to exit";
                while(!worker_m.isFinished())
                {
                    QApplication::processEvents(QEventLoop::AllEvents, 100);
                }
                QApplication::restoreOverrideCursor();
            }

            adjustWorker_m = new AdjustWorker(masterImage_m, previewImage_m, workerMutex_m);
            adjustWorker_m->moveToThread(&worker_m);
            //signal slot connections (might be able to do them in constructor?)
            connect(this, SIGNAL(distributeImageBufferAddresses(const cv::Mat*,cv::Mat*)), adjustWorker_m, SLOT(receiveImageAddresses(const cv::Mat*, cv::Mat*)));
            connect(&workSignalSuppressor, SIGNAL(suppressedSignal(SignalSuppressor*)), adjustWorker_m, SLOT(receiveSuppressedSignal(SignalSuppressor*)));
            connect(adjustWorker_m, SIGNAL(updateDisplayedImage()), this, SIGNAL(updateDisplayedImage()));
            worker_m.start();
        }

    }
    else
    {
        //while the worker event loop is running, tell it to delete itself once loop is empty.
        if(adjustWorker_m)
        {
            /* All signals to and from the object are automatically disconnected (string based, not functor),
             * and any pending posted events for the object are removed from the event queue. This is done incase functor signal/slots used later*/
            disconnect(this, SIGNAL(distributeImageBufferAddresses(const cv::Mat*,cv::Mat*)), adjustWorker_m, SLOT(receiveImageAddresses(const cv::Mat*, cv::Mat*)));
            disconnect((&workSignalSuppressor, SIGNAL(suppressedSignal(SignalSuppressor*)), adjustWorker_m, SLOT(receiveSuppressedSignal(SignalSuppressor*))));
            disconnect(adjustWorker_m, SIGNAL(updateDisplayedImage()), this, SIGNAL(updateDisplayedImage()));
            adjustWorker_m->deleteLater();
            adjustWorker_m = nullptr;
        }

    }
}
