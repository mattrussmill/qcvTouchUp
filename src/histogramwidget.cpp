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

/* HistogramWidget constructor sets the background color of the opaque paint event,
 * and initializes the histogram to have either 1 or 3 channels for an RGB uchar image
 * buffer. If another channel number is specified, histogram is initialized with 3 channels*/
HistogramWidget::HistogramWidget(QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_OpaquePaintEvent);
    backgroundColor_m = this->palette().color(QWidget::backgroundRole());
    points_m.resize(channels_m);
    histogram_m = new uint *[channels_m];
    for(int i = 0; i < channels_m; i++)
    {
        points_m[i].resize(HISTO_SIZE);
        histogram_m[i] = new uint[HISTO_SIZE];
    }
    clear();
}

/* Destructor deletes the non-Qt data buffers declared dynamically */
HistogramWidget::~HistogramWidget()
{
    while(channels_m > 0)
    {
        channels_m--;
        delete histogram_m[channels_m];
    }
    delete histogram_m;
}

/* Member function sets the specified channel color to the color argument specified if in range*/
void HistogramWidget::setChannelColor(QColor color, HistogramChannel channel)
{
    if(channel < 0 || channel > channels_m)
        return;
    if(color.isValid())
        colors_m[channel - 1] = color;
}

// Member function sets the background color of the widget to the color specified if valid.
void HistogramWidget::setBackgroundColor(QColor color)
{
    if(color.isValid())
        backgroundColor_m = color;
}

// Member function zeroizes the histogram and largest peak, then displays an empty plot.
void HistogramWidget::clear()
{
    for(int i = 0; i < channels_m; i++)
        memset(histogram_m[i], 0, HISTO_SIZE * sizeof(uint));
    largestPeak_m = 0;
    initialized_m = false;
    this->update();
}

/* An overload of clear() that clears the buffer directly passed to it by writing zeros.
 * The buffer is not checked for the correct size, use caution. */
void HistogramWidget::clear(uint **histogram, int numberOfChannels)
{
    if (!histogram || numberOfChannels < 1) return;
    for(int i = 0; i < numberOfChannels; i++)
        memset(histogram[i], 0, HISTO_SIZE * sizeof(uint));
}

/* Copies one histogram buffer into another, including all channels. Function does not check
 * for the correct buffer size of either histogra buffer, use caution */
void HistogramWidget::copy(const uint **source, uint **destination, int numberOfChannels)
{
    if (!destination || numberOfChannels < 1) return;
    for(int i = 0; i < numberOfChannels; i++)
        memcpy(destination[i], source[i], HISTO_SIZE * sizeof(uint));
}

// Sets the pen with of the histogram plot
void HistogramWidget::setLineWidth(float lineWidth)
{
    if(lineWidth > 0)
        penWidth_m = lineWidth;
}

/* Overrides the initialized state of the widget. This is useful if the internal buffer
 * is being externally managed. Remember this is reset to false on a clear event*/
void HistogramWidget::setInitialized(bool initialize)
{
    initialized_m = initialize;
}

// Sets the widget to respond to moust clicks or not
void HistogramWidget::setClickable(bool click)
{
    clickable_m = click;
}

// Returns if the widget contains a valad plot of histogram data or an empty set
bool HistogramWidget::isInitialized() const
{
    return initialized_m;
}

// Returns the clickable status of the widget
bool HistogramWidget::isClickable() const
{
    return clickable_m;
}

/* Calculates the largest value in the histogram plot. This is most often used to properly
 * scale the histogram plot to fill the widget area and scale the plot appropriately */
uint HistogramWidget::getLargestPeak() const
{
    uint s = 0;
    for(int i = 0; i < channels_m; i++)
    {
        for(int j = 0; j < HISTO_SIZE; j++)
            if( histogram_m[i][j] > s)
                s = histogram_m[i][j];
    }
    return s;
}

// Returns the number of channels in the contained histogram
int HistogramWidget::getNumberOfChannels() const
{
    return channels_m;
}

// Returns the pointer to the data containing the histogram plot and channels
uint** HistogramWidget::data() const
{
    return histogram_m;
}

/* This member function either increases or reduces the number of channels
 * that the histogram within contains.*/
void HistogramWidget::setNumberOfChannels(int numberOfChannels)
{
    if(numberOfChannels != 1 && numberOfChannels != 3)
    {
        qWarning() << "Incorrect number of channels to set.";
        return;
    }
    if(numberOfChannels < channels_m)
    {
        //remove extra channels
        while(numberOfChannels < channels_m)
        {
            delete histogram_m[channels_m - 1];
            points_m.remove(channels_m - 1);
            channels_m--;
        }
    }
    else if(numberOfChannels > channels_m)
    {
        //add new channels
        points_m.resize(numberOfChannels);
        while(numberOfChannels > channels_m)
        {
            channels_m++;
            histogram_m[channels_m - 1] = new uint[HISTO_SIZE];
            points_m[channels_m - 1].resize(HISTO_SIZE);
        }
    }
    initialized_m = false;
    clear();
}

/* This is a worker function that generates a histogram from a desired image and writes
 * the histogram data directly to the histogram buffer passed to the function. The function
 * does not check for proper allocation size or manage the histogram memory.*/
void HistogramWidget::generateHistogram(QImage &image, uint **histogram, int numberOfChannels)
{
    if(image.isNull())
    {
        qWarning() << "Image is empty.";
        return;
    }

    //zeroize buffer
    for(int i = 0; i < numberOfChannels; i++)
        memset(histogram[i], 0, HISTO_SIZE * sizeof(uint));

    //calculate histogram
    if(numberOfChannels == 3) //less operations per loop
    {
        QColor tmp;
        for(int y = 0; y < image.height(); y++)
        {
            for(int x = 0; x < image.width();x++)
            {
                tmp = image.pixel(x, y);
                histogram[Red][tmp.red()]++;
                histogram[Green][tmp.green()]++;
                histogram[Blue][tmp.blue()]++;
            }
        }
    }
    else if(numberOfChannels == 1)
    {
        for(int i = 0; i < image.width() * image.height(); i++)
        {
            histogram[0][image.bits()[i]]++;
        }
    }
    else
    {
        qWarning() << "Channel not properly set.";
        return;
    }
}

/* Deep copies a histogram of equal number of channels to the internal histogram buffer
 * and initializes the weidget to display the recently copied data*/
void HistogramWidget::setHistogramData(const uint **data, int numberOfChannels)
{ 
    if(numberOfChannels != channels_m || data == nullptr)
    {
        qWarning() << "Histogram is incorrect size.";
        return;
    }
    for(int i = 0; i < channels_m; i++)
        memcpy(histogram_m[i], data[i], HISTO_SIZE * sizeof(uint));
    initialized_m = true;
    this->update();
    emit finished();
}

/* Calculates the histogram based on the image given and stores it in the histogram member.
 * Each channel adds one (1) value per pixel intensity value and keeps a count of how many
 * pixels exist at that intensity.*/
void HistogramWidget::setHistogramData(const QImage &image)
{
    if(image.isNull())
    {
        qWarning() << "Image is empty.";
        return;
    }
    clear();

    if(channels_m == 3) //less operations per loop
    {
        QColor tmp;
        for(int y = 0; y < image.height(); y++)
        {
            for(int x = 0; x < image.width();x++)
            {
                tmp = image.pixel(x, y);
                histogram_m[Red][tmp.red()]++;
                histogram_m[Green][tmp.green()]++;
                histogram_m[Blue][tmp.blue()]++;

            }
            QApplication::processEvents();
        }
    }
    else if(channels_m == 1)
    {
        uchar overflow = 0; //faster than modulo value
        for(int i = 0; i < image.width() * image.height(); i++)
        {
            histogram_m[0][image.bits()[i]]++;
            overflow++;
            if(overflow == 0) QApplication::processEvents();
        }
    }
    else
    {
        qWarning() << "Channel not properly set.";
        return;
    }

    initialized_m = true;
    this->update();
    emit finished();
}

/* If initializes, paint event draws the lines creating the plot within the widget using
 * QPainter and painting directly on the widget surface. If the histogram has 3 channels,
 * the channels will rotate which histogram plot is displayed on top.*/
void HistogramWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.fillRect(this->rect(), backgroundColor_m);
    if(initialized_m)
    {
        //scale to account for pen edge writing outside of the surface of the widget
        calculatePoints();
        painter.setRenderHint(QPainter::Antialiasing);
        painter.scale((this->width() - penWidth_m) / this->width(),
                      (this->height() - penWidth_m) / this->height());

        QPen pen;
        pen.setStyle(Qt::SolidLine);
        pen.setJoinStyle(Qt::BevelJoin);
        pen.setCapStyle(Qt::RoundCap);
        pen.setWidthF(penWidth_m);

        if(channels_m == 3)
        {
            switch (clickState_m)
            {
            case 1:
            {
                pen.setColor(colors_m.at(Blue));
                painter.setPen(pen);
                painter.drawLines(points_m[Blue]);
                pen.setColor(colors_m.at(Red));
                painter.setPen(pen);
                painter.drawLines(points_m[Red]);
                pen.setColor(colors_m.at(Green));
                painter.setPen(pen);
                painter.drawLines(points_m[Green]);
                break;
            }
            case 2:
            {
                pen.setColor(colors_m.at(Green));
                painter.setPen(pen);
                painter.drawLines(points_m[Green]);
                pen.setColor(colors_m.at(Blue));
                painter.setPen(pen);
                painter.drawLines(points_m[Blue]);
                pen.setColor(colors_m.at(Red));
                painter.setPen(pen);
                painter.drawLines(points_m[Red]);
                break;
            }
            default:
            {
                pen.setColor(colors_m.at(Red));
                painter.setPen(pen);
                painter.drawLines(points_m[Red]);
                pen.setColor(colors_m.at(Green));
                painter.setPen(pen);
                painter.drawLines(points_m[Green]);
                pen.setColor(colors_m.at(Blue));
                painter.setPen(pen);
                painter.drawLines(points_m[Blue]);
                break;
            }
            }
        }
        else if(channels_m == 1)
        {
            pen.setColor(Qt::black);
            painter.setPen(pen);
            painter.drawLines(points_m[0]);
        }
    }
    QWidget::paintEvent(event);
}

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

/* Calculates and fills the points vector with the lines connecting the peaks of the histogram
 * for the desired channels. Scales the x and y axis to fill the widget area to be painted.*/
void HistogramWidget::calculatePoints()
{
    float yScale = this->height() / static_cast<float>(getLargestPeak());
    float xScale = this->width() / static_cast<float>(HISTO_SIZE);

    if(channels_m == 3)
    {
        for(int i = 0; i < HISTO_SIZE - 1; i++)
        {
            points_m[Red][i].setLine(i * xScale, this->height() - (histogram_m[Red][i] * yScale),
                                   (i + 1) * xScale, this->height() - (histogram_m[Red][i + 1] * yScale));
            points_m[Green][i].setLine(i * xScale, this->height() - (histogram_m[Green][i] * yScale),
                                   (i + 1) * xScale, this->height() - (histogram_m[Green][i + 1] * yScale));
            points_m[Blue][i].setLine(i * xScale, this->height() - (histogram_m[Blue][i] * yScale),
                                   (i + 1) * xScale, this->height() - (histogram_m[Blue][i + 1] * yScale));
        }
    }
    else if(channels_m == 1)
    {
        for(int i = 0; i < HISTO_SIZE - 1; i++)
        {
            points_m[0][i].setLine(i * xScale, this->height() - (histogram_m[Red][i] * yScale),
                                   (i + 1) * xScale, this->height() - (histogram_m[Red][i + 1] * yScale));
        }
    }
    //QApplication::processEvents(); ??
}
