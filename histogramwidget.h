/***********************************************************************
* FILENAME :    histogramwidget.h
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
*       Be careful passing and manipulating a histogram buffer directly.
*       Checks are not made regarding the buffer contents or if the buffer
*       itself is of the correct size.
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
#ifndef HISTOGRAMWIDGET_H
#define HISTOGRAMWIDGET_H

#include <QWidget>
#include <QVector>
class QImage;
class QColor;

#define HISTO_SIZE 256

class HistogramWidget : public QWidget
{
    Q_OBJECT   

public:
    enum HistogramChannel{Red = 0, Green = 1, Blue = 2};
    explicit HistogramWidget(QWidget *parent = nullptr);
    ~HistogramWidget();
    void setChannelColor(QColor color, HistogramChannel channel);
    void setBackgroundColor(QColor color);
    void clear();
    static void clear(uint **histogram, int numberOfChannels);
    static void copy(const uint **source, uint **destination, int numberOfChannels = 3);
    void setLineWidth(float lineWidth = 2.5);
    void setClickable(bool click);
    void setInitialized(bool initialize);
    bool isInitialized() const;
    bool isClickable() const;
    uint getLargestPeak() const;
    int getNumberOfChannels() const;
    uint** data() const;
    void setNumberOfChannels(int numberOfChannels);
    static void generateHistogram(QImage &image, uint** histogram, int numberOfChannels = 3);


signals:
    void finished();

public slots:
    void setHistogramData(const uint **data, int numberOfChannels);
    void setHistogramData(const QImage &image);


protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    void calculatePoints();
    uint **histogram_m;
    QVector<QVector<QLineF>> points_m;
    QVector<QColor> colors_m = {QColor(Qt::red), QColor(Qt::green), QColor(Qt::blue)};
    QColor backgroundColor_m;
    uint largestPeak_m;
    int clickState_m = 0;
    int channels_m = 3;
    float penWidth_m = 2.5;
    bool initialized_m = false;
    bool clickable_m = false;
};

#endif // HISTOGRAMWIDGET_H
