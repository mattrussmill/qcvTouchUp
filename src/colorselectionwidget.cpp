#include "colorselectionwidget.h"
#include "ui_colorselectionwidget.h"
#include <QPixmap>
#include <QImage>
#include <QPainter>

ColorSelectionWidget::ColorSelectionWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ColorSelectionWidget)
{
    ui->setupUi(this);

    //setup pixmaps via populating color scanlines - 7 distinct colors in Qt HSV Color Wheel (http://doc.qt.io/qt-5/qcolor.html)
    QColor initColor(Qt::lightGray);
    QImage paletteDisplay = QImage(7, 10, QImage::Format_RGB888);
    valueDisplay_m = QImage(1, 10, QImage::Format_RGB888);
    colorPreview_m.fill(Qt::lightGray);

    int hue;
    for(int x = 0; x < 7; x++)
    {
        //hue ranges from 0 - 359
        hue = (360 - (60 * x)) % 360;
        for(int y = 0; y < 10; y++)
        {
            //saturation & value set 10 y intensities to yield smoother interpolation results opposed to 2
            paletteDisplay.setPixelColor(x, y, QColor::fromHsv(hue, 255 - 255 * (y + 1) / 10.0, 255 - 40 * (y + 1) / 10.0));
            valueDisplay_m.setPixelColor(0, y, QColor::fromHsv(75, 0, 255 - 255 * (y + 1) / 10.0));
        }
    }

    //use hardware to interpolate inbetween colors
    ui->label_PaletteVisual->setPixmap(QPixmap::fromImage(paletteDisplay)
                                       .scaled(ui->label_PaletteVisual->width(), ui->label_PaletteVisual->height(),
                                               Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    ui->label_ValueVisual->setPixmap(QPixmap::fromImage(valueDisplay_m)
                                     .scaled(ui->label_ValueVisual->width(), ui->label_ValueVisual->height(),
                                             Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    ui->label_Color->setPixmap(colorPreview_m);

    //set initial menu items
    ui->verticalSlider_ValueSelect->setValue(initColor.value());
    ui->spinBox_Red->setValue(initColor.red());
    ui->spinBox_Green->setValue(initColor.green());
    ui->spinBox_Blue->setValue(initColor.blue());
    ui->spinBox_Hue->setValue(initColor.hue());
    ui->spinBox_Saturation->setValue(initColor.saturation());
    ui->spinBox_Value->setValue(initColor.value());
    ui->lineEdit_HTML->setText(initColor.name());

}

ColorSelectionWidget::~ColorSelectionWidget()
{
    delete ui;
}

void ColorSelectionWidget::setColor(QColor color)
{

}

//make virt fns
