/***********************************************************************
* FILENAME :    mouseclickdetectoreventfilter.cpp
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
#include "mouseclickdetectoreventfilter.h"
#include <QObject>
#include <QEvent>

MouseClickDetectorEventFilter::MouseClickDetectorEventFilter(QObject *parent) : QObject(parent)
{

}

/*If the intercepted event is a mouse button release, emits a signal passing true to signal detection
 * then returns false to continue event processing*/
bool MouseClickDetectorEventFilter::eventFilter(QObject *watched, QEvent *event)
{
    if(event->type() == QEvent::MouseButtonRelease)
        emit clickDetected(true);
    return false;
}
