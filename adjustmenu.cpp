#include "adjustmenu.h"
#include "ui_adjustmenu.h"
#include <cmath>
#include "mousewheeleatereventfilter.h"

#include <QDebug>

AdjustMenu::AdjustMenu(QWidget *parent) :
    QScrollArea(parent),
    ui(new Ui::AdjustMenu)
{
    ui->setupUi(this);

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

    sliderValues = new float [10];
    initializeSliders();
}

AdjustMenu::~AdjustMenu()
{
    if(!sliderValues)
        delete sliderValues;
    delete ui;
}

void AdjustMenu::initializeSliders()
{
    //disable signals from being emitted by object
    this->blockSignals(true);

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
    this->blockSignals(false);

    //Set initial parameter array for sliders
    sliderValues[0] = 0.0;      //Brightness
    sliderValues[1] = 1.0;      //Contrast
    sliderValues[2] = 255.0;      //Depth
    sliderValues[3] = 0.0;       //Hue
    sliderValues[4] = 0.0;       //Saturation
    sliderValues[5] = 0.0;       //Intensity
    sliderValues[6] = 1.0;       //Gamma
    sliderValues[7] = 1.0;       //Highlight
    sliderValues[8] = 1.0;       //Shadows
    sliderValues[9] = 1.0;        //Color
}

/* Slot signals contrast worker when triggered. When working with images represented by 8-bits per channel,
 * normalizing the contrast coefficient between 0.1 and 2.4 is preferred. To do so, the slider ranges
 * from 10 to 190 and shifts the decimal to the left before passing the alpha coefficient to the worker. */
void AdjustMenu::on_horizontalSlider_Contrast_valueChanged(int value)
{
    //if > 1, increase range from 1 to 2.4 while keeping 1 the slider center point by using log10
    if(value > 100) value *= log10(value / 10.0);
    sliderValues[1] = value / 100.0;
    emit performImageAdjustments(sliderValues);
}

/* Slot adjusts brightness slider when triggered for RGB values. It adjusts the sliders
 * between 127 and -127. Once the slider values are adjusted and the appropriate value stored in the sliderValues
 * parameter array, the signal is emitted to the image worker to perform the adjustments on the image. */
void AdjustMenu::on_horizontalSlider_Brightness_valueChanged(int value)
{
    sliderValues[0] = value;
    emit performImageAdjustments(sliderValues);
}

/* Slot adjusts the number of intensity values per channel allotted in the image. The value is adjusted on
 * an exponential curve to favor values, and have higher resolution, as the number of allotted intensity values
 * approaches 1 for the image. To the user, change is less noticeable the closer it is to 255.*/
void AdjustMenu::on_horizontalSlider_Depth_valueChanged(int value)
{
    sliderValues[2] = pow(value, value / 255.0); // difference more noticeable to the eye closer to 1
    emit performImageAdjustments(sliderValues);
}

/* Slot adjusts hue slider when triggered for HLS values. It shifts the hue value from the sliders
 * by 180 to -180 degrees. Once the slider values are adjusted and the appropriate value stored in the sliderValues
 * parameter array, the signal is emitted to the image worker to perform the adjustments on the image. */
void AdjustMenu::on_horizontalSlider_Hue_valueChanged(int value)
{
    sliderValues[3] = value;
    emit performImageAdjustments(sliderValues);
}

/* Slot adjusts saturation slider when triggered for HLS values. It adjusts the sliders
 * between 127 and -127. Once the slider values are adjusted and the appropriate value stored in the sliderValues
 * parameter array, the signal is emitted to the image worker to perform the adjustments on the image. */
void AdjustMenu::on_horizontalSlider_Saturation_valueChanged(int value)
{
    sliderValues[4] = value;
    emit performImageAdjustments(sliderValues);
}

/* Slot adjusts intensity (or lightness) slider when triggered for HLS values. It adjusts the sliders
 * between 127 and -127. Once the slider values are adjusted and the appropriate value stored in the sliderValues
 * parameter array, the signal is emitted to the image worker to perform the adjustments on the image. */
void AdjustMenu::on_horizontalSlider_Intensity_valueChanged(int value)
{
    sliderValues[5] = value;
    emit performImageAdjustments(sliderValues);
}

//Sets sliders to initial positions and signals to the worker to display the starting buffer.
void AdjustMenu::on_pushButton_Cancel_released()
{
    initializeSliders();
    emit cancelAdjustments();
}

//Sets radio button to generate a color image
void AdjustMenu::on_radioButton_Color_released()
{
    sliderValues[9] = 1.0;
    emit performImageAdjustments(sliderValues);
}

//Sets radio button to generate a grayscale image
void AdjustMenu::on_radioButton_Grayscale_released()
{
    sliderValues[9] = -1.0;
    emit performImageAdjustments(sliderValues);
}

//Sets sliders to initial positions and signals the worker to apply the changes to the master buffer.
void AdjustMenu::on_pushButton_Apply_released()
{
    initializeSliders();
    emit applyAdjustments();
}

/* Slot adjusts gamma over the whole range of intensities in the image. It adjusts the sliders between -100 and 100.
 * When the slider value is below zero it sets the gamma correction between 1 and approx 0.3. When the value is above
 * zero, it sets the gamma correction between 1 and 3.*/
void AdjustMenu::on_horizontalSlider_Gamma_valueChanged(int value)
{
    if (value < 0)
        sliderValues[6] = (value / 150.0) + 1;
    else
        sliderValues[6] = (value / 50.0) + 1;
    emit performImageAdjustments(sliderValues);
}

/* Slot shifts the gamma adjustment plot vertically between 80 and -80 for values affected between 149 and 255*/
void AdjustMenu::on_horizontalSlider_Highlight_valueChanged(int value)
{
    sliderValues[7] = value; //should use highlight method with equations provided
    emit performImageAdjustments(sliderValues);
}

/* Slot shifts the gamma adjustment plot vertically between 80 and -80 for values affected between 0 and 106*/
void AdjustMenu::on_horizontalSlider_Shadows_valueChanged(int value)
{
    sliderValues[8] = value;
    emit performImageAdjustments(sliderValues);
}

//overloads setVisible to signal the worker thread to cancel any adjustments that weren't applied when minimized
void AdjustMenu::setVisible(bool visible)
{
    if(this->isVisible() && !visible)
        emit cancelAdjustments();
    QWidget::setVisible(visible);
}
