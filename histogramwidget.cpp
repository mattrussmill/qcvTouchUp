/***********************************************************************
* FILENAME :    histogramwidget.cpp
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
*       This widget creates and manages a histogram plot of an image with
*       either 1 or 3 channels and an 8 bit-depth per channel color depth.
*       The histogram plot is drawn directly on the widget surface.
*
* NOTES :
*       QWidget was subclassed to ImageWidget instead of QScrollArea so
*       that most of the QScrollArea members remain encapsulated and
*       private.
*
*       0.2 - Expanded to include static functionality such that members
*       could be used to manipulate histogram buffers externally managed.
*
* AUTHOR :  Matthew R. Miller       START DATE :    January 01/22/2018
*
* CHANGES : N/A - N/A
*
* VERSION       DATE            WHO                     DETAIL
* 0.1           02/19/2018      Matthew R. Miller       Initial Rev
* 0.2           03/11/2018      Matthew R. Miller       Notes 0.2
*
************************************************************************/
#include "histogramwidget.h"
#include <QApplication>
#include <QWidget>
#include <QVector>
#include <QPainter>
#include <QColor>
#include <QLineF>
#include <QDebug>

//initializes the widget for fast drawing using the default background color of the widget
HistogramWidget::HistogramWidget(QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_OpaquePaintEvent);
    backgroundColor_m = this->palette().color(QWidget::backgroundRole());
    connect(&histogram_m, &HistogramObject::updated, [=](){initialized_m = true;});
    connect(&histogram_m, SIGNAL(updated()), this, SLOT(repaint()));
}

//default destructor
HistogramWidget::~HistogramWidget()
{

}

/* Member function sets the specified channel color to the color argument specified if in range (start at 1)*/
void HistogramWidget::setChannelColor(QColor color, uint channel)
{
    //if channel does not exist, extend vector to support it
    if(channel > histogram_m.getNumChannels())
        colors_m.resize(channel);
    colors_m[channel - 1] = color;
}

// Member function sets the background color of the widget to the color specified if valid.
void HistogramWidget::setBackgroundColor(QColor color)
{
    if(color.isValid())
        backgroundColor_m = color;
}

// Member function marks the histogram as clear so it will not be displayed, but leaves the data alone.
void HistogramWidget::clear()
{
    largestPeak_m = 0;
    initialized_m = false;
    this->repaint();
}

// Sets the pen with of the histogram plot
void HistogramWidget::setLineWidth(float lineWidth)
{
    if(lineWidth > 0)
        penWidth_m = lineWidth;
}

// Sets the widget to respond to moust clicks or not
void HistogramWidget::setClickable(bool click)
{
    clickable_m = click;
}

// Returns if the widget contains a valad plot of histogram data or an empty set
bool HistogramWidget::isInitialized()
{
    return initialized_m;
}

// Returns the clickable status of the widget
bool HistogramWidget::isClickable()
{
    return clickable_m;
}

// Returns the number of channels in the contained histogram
int HistogramWidget::getNumberOfChannels()
{
    return histogram_m.getNumChannels();
}

/* This is a worker function that generates a histogram from a desired image. @@ */
void HistogramWidget::setHistogramData(const QImage &image)
{
    //@@ don't forget to make this a worker task afterwards & downsample of image
    histogram_m.update(image);
}

///* If initializes, paint event draws the lines creating the plot within the widget using
// * QPainter and painting directly on the widget surface. If the histogram has 3 channels,
// * the channels will rotate which histogram plot is displayed on top.*/
//void HistogramWidget::paintEvent(QPaintEvent *event)
//{
//    QPainter painter(this); //shouldn't recreate this every paint event?
//    painter.fillRect(this->rect(), backgroundColor_m);
//    if(initialized_m)
//    {
//        //scale to account for pen edge writing outside of the surface of the widget
//        calculatePoints();
//        painter.setRenderHint(QPainter::Antialiasing);
//        painter.scale((this->width() - penWidth_m) / this->width(),
//                      (this->height() - penWidth_m) / this->height());

//        QPen pen;
//        pen.setStyle(Qt::SolidLine);
//        pen.setJoinStyle(Qt::BevelJoin);
//        pen.setCapStyle(Qt::RoundCap);
//        pen.setWidthF(penWidth_m);

//        if(channels_m == 3)
//        {
//            switch (clickState_m)
//            {
//            case 1:
//            { //@@ have 3 different images (1 for each channel) and paint them in a separate iterative order based on
//                pen.setColor(colors_m.at(Blue));
//                painter.setPen(pen);
//                painter.drawLines(points_m[Blue]);
//                pen.setColor(colors_m.at(Red));
//                painter.setPen(pen);
//                painter.drawLines(points_m[Red]);
//                pen.setColor(colors_m.at(Green));
//                painter.setPen(pen);
//                painter.drawLines(points_m[Green]);
//                break;
//            }
//            case 2:
//            {
//                pen.setColor(colors_m.at(Green));
//                painter.setPen(pen);
//                painter.drawLines(points_m[Green]);
//                pen.setColor(colors_m.at(Blue));
//                painter.setPen(pen);
//                painter.drawLines(points_m[Blue]);
//                pen.setColor(colors_m.at(Red));
//                painter.setPen(pen);
//                painter.drawLines(points_m[Red]);
//                break;
//            }
//            default:
//            {
//                pen.setColor(colors_m.at(Red));
//                painter.setPen(pen);
//                painter.drawLines(points_m[Red]);
//                pen.setColor(colors_m.at(Green));
//                painter.setPen(pen);
//                painter.drawLines(points_m[Green]);
//                pen.setColor(colors_m.at(Blue));
//                painter.setPen(pen);
//                painter.drawLines(points_m[Blue]);
//                break;
//            }
//            }
//        }
//        else if(channels_m == 1)
//        {
//            pen.setColor(Qt::black);
//            painter.setPen(pen);
//            painter.drawLines(points_m[0]);
//        }
//    }
//    QWidget::paintEvent(event);
//}

// Updates the click state machine for drawing histogram plots in the correct layer order.
void HistogramWidget::mousePressEvent(QMouseEvent *event)
{
    if(clickable_m)
    {
        clickState_m++;
        if(clickState_m > 2)
            clickState_m = 0;
        update();
    }
    QWidget::mousePressEvent(event);
}

///* Calculates and fills the points vector with the lines connecting the peaks of the histogram
// * for the desired channels. Scales the x and y axis to fill the widget area to be painted.*/
//void HistogramWidget::calculatePoints()
//{
//    float yScale = this->height() / static_cast<float>(getLargestPeak());
//    float xScale = this->width() / static_cast<float>(HISTO_SIZE);

//    if(channels_m == 3)
//    {
//        for(int i = 0; i < HISTO_SIZE - 1; i++)
//        {
//            points_m[Red][i].setLine(i * xScale, this->height() - (histogram_m[Red][i] * yScale),
//                                   (i + 1) * xScale, this->height() - (histogram_m[Red][i + 1] * yScale));
//            points_m[Green][i].setLine(i * xScale, this->height() - (histogram_m[Green][i] * yScale),
//                                   (i + 1) * xScale, this->height() - (histogram_m[Green][i + 1] * yScale));
//            points_m[Blue][i].setLine(i * xScale, this->height() - (histogram_m[Blue][i] * yScale),
//                                   (i + 1) * xScale, this->height() - (histogram_m[Blue][i + 1] * yScale));
//        }
//    }
//    else if(channels_m == 1)
//    {
//        for(int i = 0; i < HISTO_SIZE - 1; i++)
//        {
//            points_m[0][i].setLine(i * xScale, this->height() - (histogram_m[Red][i] * yScale),
//                                   (i + 1) * xScale, this->height() - (histogram_m[Red][i + 1] * yScale));
//        }
//    }
//    //QApplication::processEvents(); ??
//}
