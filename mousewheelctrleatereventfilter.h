#ifndef MOUSEWHEELCTRLEATEREVENTFILTER_H
#define MOUSEWHEELCTRLEATEREVENTFILTER_H

#include <QObject>

class MouseWheelCtrlEaterEventFilter : public QObject
{
    Q_OBJECT
public:
    explicit MouseWheelCtrlEaterEventFilter(QObject *parent = nullptr);

signals:

public slots:
};

#endif // MOUSEWHEELCTRLEATEREVENTFILTER_H
