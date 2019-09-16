#include "transformimagewidgetcropeventfilter.h"
#include <QObject>
#include <QEvent>
#include "imagewidget.h"

TransformImageWidgetCropEventFilter::TransformImageWidgetCropEventFilter(QObject *parent) : QObject(parent)
{
    initializePaintMembers();

}

//eventFilter returns true to disable further processing ....
bool TransformImageWidgetCropEventFilter::eventFilter(QObject *watched, QEvent *event)
{
    //continue with normal processing if no image attached
    if(!imageWidget_m->imageAttached())
        return false;

    switch(event->type())
    {
        case QEvent::Paint:
            break;

        case QEvent::MouseButtonPress:
            break;

        case QEvent::MouseButtonRelease:
            //mouseReleasedActoions(event*)?
            break;

        case QEvent::MouseMove:
            break;
    }

}

void TransformImageWidgetCropEventFilter::setPointFromImage()
{

}

//initializes the member variables used for painting on the pixmap
void TransformImageWidgetCropEventFilter::initializePaintMembers()
{
    dragStart_m = QPoint(-1, -1);
    region_m = QRect(dragStart_m, dragStart_m);
    brushRadius_m = 0;
}
