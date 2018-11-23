/***********************************************************************
* FILENAME :    signalsuppressor.h
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
*       This object should be used when passing data between threads using a
*       queued signal/slot connection in Qt where a fast thread is signaling
*       a slower worker thread (fast thread -> slow thread) if the slower worker
*       thread cannot keep up with requests and lags behind when processing
*       the queued signals from the faster thread. This object allows for only
*       the most recently signalled data to make it to the worker thread once
*       the work has completed.
*
* NOTES :
*       The passed data must exist as part of QVariant and the receiving slot
*       must mimic the following pseudo code:
*       RECEIVE_SUPPRESSOR(SignalSuppressor *ptr)
*       {
*          ...
*          data = ptr->getNewData().toDATATYPE();
*          performWork(data);
*       }
*
*       Signaling to the SignalSuppressor must also be done via the new signaling
*       method mimicing the following pseudo code:
*       connect(*signalingObj, &Class::method, thisObj, SignalSuppressor::receiveNewData);
*
*
* AUTHOR :  Matthew R. Miller       START DATE :    January 11/23/2018
*
* CHANGES : N/A - N/A
*
* VERSION       DATE            WHO                     DETAIL
* 0.1           11/23/2018      Matthew R. Miller       Initial Rev
*
************************************************************************/
#ifndef SIGNALSUPPRESSOR_H
#define SIGNALSUPPRESSOR_H

#include <QObject>
#include <QVariant>
#include <QMutex>

class SignalSuppressor : public QObject
{
    Q_OBJECT
public:
    explicit SignalSuppressor(QObject *parent = nullptr);
    QVariant getNewData();

signals:
    void suppressedSignal(SignalSuppressor *ptr);

public slots:
    void receiveNewData(QVariant newData);

private:
    QVariant data_m;
    QMutex mutex;
    bool notWaitingForData = true;
};

#endif // SIGNALSUPPRESSOR_H
