#include "colorselectionwidget.h"
#include "ui_colorselectionwidget.h"
#include <cmath>
#include <QPixmap>
#include <QImage>
#include <QPainter>

ColorSelectionWidget::ColorSelectionWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ColorSelectionWidget)
{
    ui->setupUi(this);

    //setup color scanlines - 7 distinct colors in Qt HSV Color Wheel (http://doc.qt.io/qt-5/qcolor.html)
    QImage paletteDisplay = QImage(7, 10, QImage::Format_RGB888);
    valueDisplay_m = QImage(1, 10, QImage::Format_RGB888);

    int hue, saturation, value;
    for(int x = 0; x < 7; x++)
    {
        //hue ranges from 0 - 359
        hue = (360 - (60 * x)) % 360;
        for(int y = 0; y < 10; y++)
        {
            //saturation & value adjusted on log10 scale so it is more accurate to human perception of light intensity
            saturation = 255 - 255 * log10(y + 1);
            value = 255 - 40 * log10(y + 1);
            paletteDisplay.setPixelColor(x, y, QColor::fromHsv(hue, saturation, value));
            //valueDisplay_m.setPixelColor(0, y, QColor::fromHsv(hue, saturation, value));
        }
    }

    //use hardware to interpolate inbetween colors
    ui->label_PaletteVisual->setPixmap(QPixmap::fromImage(paletteDisplay)
                                       .scaled(ui->label_PaletteVisual->width(), ui->label_PaletteVisual->height(),
                                               Qt::IgnoreAspectRatio, Qt::SmoothTransformation));

}

ColorSelectionWidget::~ColorSelectionWidget()
{
    delete ui;
}


//make virt fns
