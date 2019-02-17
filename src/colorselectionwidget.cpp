/***********************************************************************
* FILENAME :    colorselectionwidget.cpp
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
*       This is a compact, widget version of QColorDialog. It is a graphical color
*       selection widget which emits a signal for a desired color when any internal
*       widget is updated which changes the value of the color.
*
* NOTES :
*       This object was created because QColorDialog was difficult to embed into
*       another widget with no real way of removing some of its widgets.
*
* AUTHOR :  Matthew R. Miller       START DATE :    January 01/28/2019
*
* CHANGES : N/A - N/A
*
* VERSION       DATE            WHO                     DETAIL
* 0.1           02/17/2019      Matthew R. Miller       Initial Rev
*
************************************************************************/
#include "colorselectionwidget.h"
#include "mousewheeleatereventfilter.h"
#include "ui_colorselectionwidget.h"
#include <QPixmap>
#include <QImage>
#include <QPainter>
#include <QMouseEvent>
#include <QPoint>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

/* The constructor first installs any event filters. Then by populating a small QImage for the color
 * palette picker and shifting the HSV values appropriately, it uses hardware acceleration to interpolate
 * the "in-between" values for the larger representation of the color selection palette for selecting
 * Hue and Saturation values keeping the memory footprint lower and loating times down. Lastly all
 * appropriate signals and slots are connected.*/
ColorSelectionWidget::ColorSelectionWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ColorSelectionWidget)
{
    ui->setupUi(this);
    MouseWheelEaterEventFilter *wheelFilter = new MouseWheelEaterEventFilter(this);
    ui->verticalSlider_ValueSelect->installEventFilter(wheelFilter);

    //setup pixmaps via populating color scanlines - 7 distinct colors in Qt HSV Color Wheel (http://doc.qt.io/qt-5/qcolor.html)
    QImage paletteDisplay = QImage(7, 10, QImage::Format_RGB888);
    selectedColor_m = QColor(Qt::lightGray);
    valueDisplay_m = QImage(1, 10, QImage::Format_RGB888);

    //generate palette
    int hue;
    for(int x = 0; x < 7; x++)
    {
        //hue ranges from 0 - 359
        hue = (360 - (60 * x)) % 360;
        for(int y = 0; y < 10; y++)
        {
            //saturation & value set 10 y intensities to yield smoother interpolation results opposed to 2
            paletteDisplay.setPixelColor(x, y, QColor::fromHsv(hue, 255 - 255 * (y + 1) / 10.0, 255 - 40 * (y + 1) / 10.0));
        }
    }

    //use hardware to interpolate inbetween colors
    ui->label_PaletteVisual->setPixmap(QPixmap::fromImage(paletteDisplay)
                                       .scaled(ui->label_PaletteVisual->width(), ui->label_PaletteVisual->height(),
                                               Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    ui->label_PaletteVisual->setCursor(Qt::CrossCursor);

    //set initial menu items
    populateColorValues();

    //connect signals and slots
    connect(ui->verticalSlider_ValueSelect, SIGNAL(valueChanged(int)), this, SLOT(setColorFromPalette()));
    connect(ui->spinBox_Red, SIGNAL(valueChanged(int)), this, SLOT(setColorFromRGB()));
    connect(ui->spinBox_Green, SIGNAL(valueChanged(int)), this, SLOT(setColorFromRGB()));
    connect(ui->spinBox_Blue, SIGNAL(valueChanged(int)), this, SLOT(setColorFromRGB()));
    connect(ui->spinBox_Hue, SIGNAL(valueChanged(int)), this, SLOT(setColorFromHSV()));
    connect(ui->spinBox_Saturation, SIGNAL(valueChanged(int)), this, SLOT(setColorFromHSV()));
    connect(ui->spinBox_Value, SIGNAL(valueChanged(int)), this, SLOT(setColorFromHSV()));
    connect(ui->lineEdit_HTML, SIGNAL(textEdited(QString)), this, SLOT(setColorFromHTML()));
}

//autogenerated destructor
ColorSelectionWidget::~ColorSelectionWidget()
{
    delete ui;
}

/* If the mouse release event is left click and within the palette selection widget the appropriate
 * method is called to select the hue and saturation values from the palette graphic.*/
void ColorSelectionWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        QPoint mousePosition = ui->label_PaletteVisual->mapFromParent(event->pos());
        if(mousePosition.x() >= 0 && mousePosition.y() >= 0
                && mousePosition.x() < ui->label_PaletteVisual->width()
                && mousePosition.y() < ui->label_PaletteVisual->height())
        {
            palettePoint_m = mousePosition;
            //paint pixmap
            setColorFromPalette();

            //emit color
        }
    }
    QWidget::mouseReleaseEvent(event);
}

/* If the mouse move event is within the palette selection widget the appropriate
 * method is called to select the hue and saturation values from the palette graphic.*/
void ColorSelectionWidget::mouseMoveEvent(QMouseEvent *event)
{
    if(!event->pos().isNull())
    {
        QPoint mousePosition = ui->label_PaletteVisual->mapFromParent(event->pos());
        if(mousePosition.x() >= 0 && mousePosition.y() >= 0
                && mousePosition.x() < ui->label_PaletteVisual->width()
                && mousePosition.y() < ui->label_PaletteVisual->height())
        {
            palettePoint_m = mousePosition;
            //paint pixmap
            setColorFromPalette();

        }
    }
    QWidget::mouseMoveEvent(event);
}

/* If the mouse press event is left click and within the palette selection widget the appropriate
 * method is called to select the hue and saturation values from the palette graphic.*/
void ColorSelectionWidget::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        QPoint mousePosition = ui->label_PaletteVisual->mapFromParent(event->pos());
        if(mousePosition.x() >= 0 && mousePosition.y() >= 0
                && mousePosition.x() < ui->label_PaletteVisual->width()
                && mousePosition.y() < ui->label_PaletteVisual->height())
        {
            palettePoint_m = mousePosition;
            //paint pixmap
            setColorFromPalette();

        }
    }
    QWidget::mousePressEvent(event);
}

//this member function sets the selected color of the widget from an outside source.
void ColorSelectionWidget::setColor(const QColor *color)
{
    selectedColor_m = *color;
    setCursorFromColor();
}

/* setColorFromPalette is a private slot that is meant to be called during a mouse event when a button is depressed.
 * After calculating the ratio between the width and height of the color palette clicked on and its corresponding hue
 * and saturation level, this calculates the color value of the HSV color model based on the point clicked in the palette
 * stored as a member variable. Then this calls the method to populate the value color bar and color text boxes.*/
void ColorSelectionWidget::setColorFromPalette()
{
    //remember y axes is inverted because qimage and x is because hue phase shift reversed
    int hue = 359 - 359 * palettePoint_m.x() / static_cast<float>(ui->label_PaletteVisual->width() - 1);
    int saturation = 255 - 255 * palettePoint_m.y() / static_cast<float>(ui->label_PaletteVisual->height() - 1);
    selectedColor_m = QColor::fromHsv(hue, saturation, ui->verticalSlider_ValueSelect->value());
    populateColorValues();
}

/* This method populates the Value display and all other widget values and their position based on the color value
 * stored internally in the private member selectedColor_m. This should be called every time that value is updated
 * so all widgets reflect the correct value corresponding to the color selected.*/
void ColorSelectionWidget::populateColorValues()
{
    //see comments in constructor for value-bar generation
    for(int y = 0; y < 10; y++)
    {
        valueDisplay_m.setPixelColor(0, y, QColor::fromHsv(selectedColor_m.hue(), 255, 255 - 255 * (y + 1) / 10.0));
    }
    ui->label_ValueVisual->setPixmap(QPixmap::fromImage(valueDisplay_m)
                                     .scaled(ui->label_ValueVisual->width(), ui->label_ValueVisual->height(),
                                             Qt::IgnoreAspectRatio, Qt::SmoothTransformation));

    QImage color(1, 1, QImage::Format_RGB888);
    color.setPixelColor(0, 0, selectedColor_m);
    ui->label_Color->setPixmap(QPixmap::fromImage(color).
                               scaled(ui->label_Color->width(), ui->label_Color->height(),
                                      Qt::IgnoreAspectRatio, Qt::FastTransformation));

    ui->spinBox_Red->blockSignals(true);
    ui->spinBox_Green->blockSignals(true);
    ui->spinBox_Blue->blockSignals(true);
    ui->spinBox_Hue->blockSignals(true);
    ui->spinBox_Saturation->blockSignals(true);
    ui->spinBox_Value->blockSignals(true);
    ui->verticalSlider_ValueSelect->blockSignals(true);

    ui->spinBox_Red->setValue(selectedColor_m.red());
    ui->spinBox_Green->setValue(selectedColor_m.green());
    ui->spinBox_Blue->setValue(selectedColor_m.blue());
    ui->spinBox_Hue->setValue(selectedColor_m.hue());
    ui->spinBox_Saturation->setValue(selectedColor_m.saturation());
    ui->spinBox_Value->setValue(selectedColor_m.value());
    ui->verticalSlider_ValueSelect->setValue(selectedColor_m.value());
    ui->lineEdit_HTML->setText(selectedColor_m.name());

    ui->spinBox_Red->blockSignals(false);
    ui->spinBox_Green->blockSignals(false);
    ui->spinBox_Blue->blockSignals(false);
    ui->spinBox_Hue->blockSignals(false);
    ui->spinBox_Saturation->blockSignals(false);
    ui->spinBox_Value->blockSignals(false);
    ui->verticalSlider_ValueSelect->blockSignals(false);

    emit colorSelected(selectedColor_m);
}

/* setColorFromRGB is a private slot that sets the internal color value based on an updated RGB value from the designated
 * RGB spinboxes in the UI, then updates the dependent member variables for the widget.*/
void ColorSelectionWidget::setColorFromRGB()
{
    selectedColor_m = QColor::fromRgb(ui->spinBox_Red->value(), ui->spinBox_Green->value(), ui->spinBox_Blue->value());
    populateColorValues();
    setCursorFromColor();
}

/* setColorFromHSV is a private slot that sets the internal color value based on an updated HSV value from the designated
 * HSV spinboxes in the UI, then updates the dependent member variables for the widget.*/
void ColorSelectionWidget::setColorFromHSV()
{
    selectedColor_m = QColor::fromHsv(ui->spinBox_Hue->value(), ui->spinBox_Saturation->value(), ui->spinBox_Value->value());
    populateColorValues();
    setCursorFromColor();
}

/* setColorFromHTML is a private slot that sets the color (and dependent member variables) based on the HTML color code typed
 * into the appropriate line edit only if the format first matches the #xxxxxx color format. If it does not match then the
 * color box on the widget displays "Invalid" to notify the user that the desired input is not the correct format.*/
void ColorSelectionWidget::setColorFromHTML()
{
    QRegularExpression re("^#[0-9a-fA-F]{6}$");
    QRegularExpressionMatch colorCode = re.match(ui->lineEdit_HTML->text());

    if(colorCode.hasMatch())
    {
        selectedColor_m.setNamedColor(colorCode.captured());
        populateColorValues();
        setCursorFromColor();
    }
    else
    {
        ui->label_Color->setText("Invalid");
    }
}

/* setCursorFromColor sets the member variable paletPoint_m to the point on the palette based on the selectedColor_m value.
 * This is specifically useful when the spinboxes are used to set the color instead of the palette because calling
 * setColorFromPalette right after adjusting the color from the spinboxes uses the palettePoint_m member which is the last
 * clicked value if not updated. This would cause inconsistencies in color selection.*/
void ColorSelectionWidget::setCursorFromColor()
{
    int sizex = ui->label_PaletteVisual->width() - 1;
    int x = sizex - sizex * selectedColor_m.hue() / 359.0;

    int sizey = ui->label_PaletteVisual->height() - 1;
    int y = sizey - sizey * selectedColor_m.saturation() / 255.0;

    palettePoint_m.setX(x);
    palettePoint_m.setY(y);
}
