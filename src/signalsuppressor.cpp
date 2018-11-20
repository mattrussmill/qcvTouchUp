/* Used for passing qt signals between threads if the worker cannot keep up with
 * requests. It may drop data and only send the most updated data to the worker (reword this)
 *
 *
 * IN PROGRESS
 */

#include "signalsuppressor.h"
#include <QPoint>
#include <QMutex>

SignalSuppressor::SignalSuppressor(QObject *parent) : QObject(parent)
{

}

//deletes the dynamically allocated data before object is destroyed
SignalSuppressor::~SignalSuppressor()
{
    data_m.clear();
}

/* receiveNewData takes new data from a signal and places it on the internal
 * data structure to the object. If the buffer exceeds a particular size, the
 * buffer is cleared to keep growth from becoming unmanageable.*/
void SignalSuppressor::receiveNewData(QVariant newData)
{
    if(data_m.size() > maxBuffer_m)
        deleteOldData();
    data_m.push(newData);
}

/* retrieveNewData takes a parameter of the last data used by the calling object
 * and compares it to the most recent item inserted into the buffer. If it is not
 * a match, the most recent addition to the buffer is returned and the buffer is
 * cleared. If the data is the same or the buffer is empty, a null QVariant is
 * returned*/
QVariant SignalSuppressor::retrieveNewData(QVariant oldData)
{
    deleteOldData();
    if(oldData == data_m.top() || data_m.isEmpty())
        return QVariant();
    return data_m.top();

}

//returns the buffersize of the signal suppressor
uint16_t SignalSuppressor::getBufferSize()
{
    return maxBuffer_m;
}

//sets the buffer size for the suppression buffer
void SignalSuppressor::setBufferSize(uint16_t size)
{
    maxBuffer_m = size;
}

void SignalSuppressor::deleteOldData()
{

}



