#include "transformimagewidgetcropeventfilter.h"
#include <QObject>
#include <QEvent>

TransformImageWidgetCropEventFilter::TransformImageWidgetCropEventFilter(QObject *parent) : QObject(parent)
{

}

//eventFilter returns true to disable further processing ....
bool TransformImageWidgetCropEventFilter::eventFilter(QObject *watched, QEvent *event)
{

}
