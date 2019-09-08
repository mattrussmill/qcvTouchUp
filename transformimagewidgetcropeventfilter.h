//started 9/7/19 // install directly to ImageLabel -> Have slot to "installEventFilterToImage()" or something, then have removeEventFilterFromImageLabel() to be called during radio switch context etc.

#ifndef TRANSFORMIMAGEWIDGETCROPEVENTFILTER_H
#define TRANSFORMIMAGEWIDGETCROPEVENTFILTER_H

#include <QObject>
#include <QPoint>
class QLabel;
class QPoint;

class TransformImageWidgetCropEventFilter : public QObject
{
    Q_OBJECT
public:
    explicit TransformImageWidgetCropEventFilter(QObject *parent = nullptr);

signals:

public slots:

protected:
    bool eventFilter(QObject *watched, QEvent *event);

private:
    //mouseMoveEvent, mousePressEvent, mouseReleaseEvent, paintEvent
    //call redraw after each mouse event
    //initializePaintMembers in constructor
    //selectRegionOnPixmap

    void setPointFromImage(); //getPointInImage from ImageWidget
    QPoint pointInImage_m; //store pointInImage here to not reallocate every time
    QPoint dragStart_m;
    QLabel *ImageWidget;



};

#endif // TRANSFORMIMAGEWIDGETCROPEVENTFILTER_H
