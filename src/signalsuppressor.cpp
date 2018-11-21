/* Used for passing qt signals between threads if the worker cannot keep up with
 * requests. It may drop data and only send the most updated data to the worker (reword this)
 *
 * //fast thread -> slow thread communication (stack type operation while throwing out the rest)
 *
 * IN PROGRESS
 */

#include "signalsuppressor.h"
#include <QDebug>

SignalSuppressor::SignalSuppressor(QObject *parent) : QObject(parent)
{

}

/* receiveNewData takes new data from a signal (signaling thread / fast thread) and places it in the internal
 * container of the object while protecting it from a race condition via a mutex. The old data is overwritten
 * every time a new signal is sent. If init, or the data has been retrieved, data_m should be reset to !Valid
 * so that a signal is only emitted (to the receiving thread / slow thread) only when new data is ready and only
 * one event is queued.*/
void SignalSuppressor::receiveNewData(QVariant newData)
{
    if(!data_m.isValid())
    {
        emit suppressedSignal(this);
        qDebug() << "suppressed signal activated";
    }
    mutex.lock();
    data_m = newData;
    mutex.unlock();
} //disconnecting signal / slot not an option as last value may be dropped during processing



/* retrieveNewData takes a parameter of the last data used by the calling object in the recieving thread
 * (this method should ONLY be called and run in the receiving / slow thread) and compares it to the most
 * recent item inserted into this object.
 *
 * If the old and new data does not match the new data is sent,
 * else an invalid QVarient is sent in its place. When called the data member is reinitialized to !Valid*/
QVariant SignalSuppressor::retrieveNewData(QVariant oldData) //fn ptr should go here and run according to notes
{
    mutex.lock();
    QVariant tmp = data_m;
    data_m = QVariant();
    mutex.unlock();

    if(tmp != oldData)
        return tmp;
    return QVariant();

} /*template <class T, int N>
void mysequence<T,N>::setmember (int x, T value) {
  memblock[x]=value;
}*/



