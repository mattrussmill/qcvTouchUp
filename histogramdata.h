/***********************************************************************
* FILENAME :    histogramdata.h
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
*       This object contains the implicitly shared buffer which houses the
*       histogram data for an image.
*
* NOTES :
*       This should only hold/maintain the implicitly shared data structure.
*
* AUTHOR :  Matthew R. Miller       START DATE :    June 6, 2019
*
* CHANGES : N/A - N/A
*
* VERSION       DATE                    WHO                     DETAIL
* 0.1           June 12, 2019       Matthew R. Miller       Initial Rev
*
************************************************************************/

#ifndef HISTOGRAMDATA_H
#define HISTOGRAMDATA_H

#include <QSharedData>
#include <QVector>

class HistogramData : public QSharedData
{

public:
    HistogramData();
    HistogramData(const HistogramData &object);
    ~HistogramData();

    QVector<uint32_t> histogram;
};

#endif // HISTOGRAMDATA_H
