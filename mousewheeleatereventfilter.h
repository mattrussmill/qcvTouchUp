/***********************************************************************
* FILENAME :    mousewheeleatereventfilter.h
*
* DESCRIPTION :
*       This object is an event filter to disable mouse scroll wheel interactions.
*
* NOTES :
*       Some QWidgets have scroll wheel interactions which cannot be disabled.
*
* AUTHOR :  Matthew R. Miller       START DATE :    March 6, 2018
*
* CHANGES : N/A - N/A
*
* VERSION       DATE            WHO                     DETAIL
* 0.1           March 6, 2018      Matthew R. Miller       Initial Rev
*
************************************************************************/
#ifndef MOUSEWHEELEATEREVENTFILTER_H
#define MOUSEWHEELEATEREVENTFILTER_H

#include <QObject>

class MouseWheelEaterEventFilter : public QObject
{
    Q_OBJECT
public:
    explicit MouseWheelEaterEventFilter(QObject *parent = nullptr);

protected:
    bool eventFilter(QObject *watched, QEvent *event);
};

#endif // MOUSEWHEELEATEREVENTFILTER_H
