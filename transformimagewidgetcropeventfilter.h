//started 9/7/19 // install directly to ImageLabel -> Have slot to "installEventFilterToImage()" or something, then have removeEventFilterFromImageLabel() to be called during radio switch context etc.

#ifndef TRANSFORMIMAGEWIDGETCROPEVENTFILTER_H
#define TRANSFORMIMAGEWIDGETCROPEVENTFILTER_H

#include <QObject>
#include <QPoint>
#include <QRect>
class QLabel;
class ImageWidget;

class TransformImageWidgetCropEventFilter : public QObject
{
    Q_OBJECT
public:
    explicit TransformImageWidgetCropEventFilter(QObject *parent = nullptr);
    enum CoordinateMode
    {
        NoClick        = 0x0,
        SingleClick    = 0x1,
        SingleUnclick  = 0x2,
        ClickUnclick   = 0x4,
        ClickDrag      = 0x8,
        RectROI        = 0x10,
        DragROI        = 0x20,
        BrushImage     = 0x40
    };

signals:
    void imagePointSelected(QPoint selectedPoint);

public slots:
    void initializePaintMembers();
    void setScale(float scale);

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
    QRect region_m;
    uchar brushRadius_m;
    float scale_m;
    uint retrieveCoordinateMode_m = NoClick;
    ImageWidget *imageWidget_m; //cast from parent to install other signals or slots -> so all are taken care of from installing to ImageLabel
    QLabel *imageLabel_m;



};

#endif // TRANSFORMIMAGEWIDGETCROPEVENTFILTER_H
