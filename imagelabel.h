#ifndef IMAGELABEL_H
#define IMAGELABEL_H

#include <QLabel>

class ImageLabel : public QLabel
{
    Q_OBJECT

public:
    ImageLabel(QWidget *parent = nullptr);
    uint getRetrieveCoordinateMode();
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
    void imageRectRegionSelected(QRect roi);

public slots:
    void initializePaintMembers();
    void setRetrieveCoordinateMode(uint mode);
    void setRectRegionSelected(QRect roi);

protected:
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void paintEvent(QPaintEvent *event) override;
    virtual void resizeEvent(QResizeEvent *event) override;

private:
    QPoint getPointInImage(QMouseEvent *event);
    QRect getAdjustedRegion();
    void leftMousePressEvent(QMouseEvent *event);
    void leftMouseMoveEvent(QMouseEvent *event);
    void leftMouseReleaseEvent(QMouseEvent *event);
    void selectRegionOnPixmap();


    void setPointFromImage(); //getPointInImage from ImageWidget
    QPoint pointInImage_m; //store pointInImage here to not reallocate every time
    QPoint dragStart_m;
    QRect region_m;
    uchar brushRadius_m;
    float scaleWidth_m = 0;
    float scaleHeight_m = 0;
    uint retrieveCoordinateMode_m = NoClick;
};

#endif // IMAGELABEL_H
