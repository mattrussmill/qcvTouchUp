/***********************************************************************
* FILENAME :    signalsuppressor.cpp
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
#include "signalsuppressor.h"
#include <QDebug>

SignalSuppressor::SignalSuppressor(QObject *parent) : QObject(parent)
{

}

/* receiveNewData takes new data from a signal (signaling thread / fast thread) and places it in the internal
 * container of the object while protecting it from a race condition via a mutex. The old data is overwritten
 * every time a new signal is sent. The signal is only emitted if the receiving (slow) thread has pulled (getNewData)
 * the data from the object. The internal flag is reset to release the signal the first instant more data is received.*/
void SignalSuppressor::receiveNewData(QVariant newData)
{
    mutex.lock();
    data_m = newData;
    if(notWaitingForData)
    {
        notWaitingForData = false;
        emit suppressedSignal(this);
        qDebug() << "suppressed signal ACTIVATED";
    }
    mutex.unlock();
} //disconnecting signal / slot not an option as last value may be dropped during processing


/* getNewData should be called by the receiving thread which has received this object's address through
 * a queued signal/slot connection. The slow thread slot should call this function to retrieve the most
 * recent data it has been passed, ignoring any other data being passed between work iterations.  */
QVariant SignalSuppressor::getNewData()
{
    mutex.lock();
    notWaitingForData = true; //data has been retrieved. Next newest data will send one signal and wait
    QVariant tmp = data_m;
    mutex.unlock();
    return tmp;
}



