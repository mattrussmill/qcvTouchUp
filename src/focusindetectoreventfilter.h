/***********************************************************************
* FILENAME :    mouseclickdetectoreventfilter.h
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
*       This object emits a signal passing a bool that is true whenever a mouse
*       button release event occurs. It does not interrupt further processing of
*       the event.
*
* NOTES :
*       Some QWidgets have scroll wheel interactions which cannot be disabled.
*
* AUTHOR :  Matthew R. Miller       START DATE :    March 6, 2018
*
* CHANGES : N/A - N/A
*
* VERSION       DATE                    WHO                     DETAIL
* 0.1           December 19, 2018       Matthew R. Miller       Initial Rev
*
************************************************************************/

#ifndef FOCUSINDETECTOREVENTFILTER_H
#define FOCUSINDETECTOREVENTFILTER_H

#include <QObject>

class FocusInDetectorEventFilter : public QObject
{
    Q_OBJECT
public:
    explicit FocusInDetectorEventFilter(QObject *parent = nullptr);

signals:
    void focusDetected(bool);

protected:
    bool eventFilter(QObject *watched, QEvent *event);

public slots:
};

#endif // FOCUSINDETECTOREVENTFILTER_H