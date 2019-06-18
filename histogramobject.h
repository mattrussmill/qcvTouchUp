/***********************************************************************
* FILENAME :    histogramobject.h
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
*       This object maintains and calculates a multi-channel histogram for an
*       image, including handles all of the shared data operations for implicit
*       histogram buffer sharing.
*
*
* AUTHOR :  Matthew R. Miller       START DATE :    June 6, 2019
*
* CHANGES : N/A - N/A
*
* VERSION       DATE                    WHO                     DETAIL
* 0.1           June 17, 2019       Matthew R. Miller       Initial Rev
*
************************************************************************/
#ifndef HISTOGRAMOBJECT_H
#define HISTOGRAMOBJECT_H

#include <QObject>
#include <QSharedData>
#include <QSharedDataPointer>
class QImage;
class HistogramData;

class HistogramObject : public QObject
{
    Q_OBJECT

public:
    HistogramObject();
    HistogramObject(const QImage &image);
    HistogramObject(const HistogramObject &hobj);
    ~HistogramObject();
    HistogramObject clone();
    uint32_t operator[](int index);
    int getIndex(int channel, int pixelIntensity);
    int getNumChannels();

public slots:
    bool update(const QImage &image);

signals:
    void updated();

private:
    QSharedDataPointer<HistogramData> data_m;
};

#endif // HISTOGRAMOBJECT_H
