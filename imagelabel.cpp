#include "imagelabel.h"
#include <QMouseEvent>
#include <QPainter>
#include <QDebug>

ImageLabel::ImageLabel(QWidget *parent) : QLabel(parent)
{
    initializePaintMembers();
}

/* An override of mouseReleaseEvent. If an image is attached and the left button is released
 * overtop of the ImageWidget while an image is present and the appropriate selectPixelMode_m
 * selected, the coordinates under the mouse are emitted as a QPoint relative to the
 * attachedImage_m's origin. If RectROI is selected, the region is finished being set, cropped
 * and drawn before being emitted, else if DragROI the region is only cropped to fit and emitted. */
void ImageLabel::mouseReleaseEvent(QMouseEvent *event)
{
    if(this->pixmap() != nullptr)
    {
        if(event->button() == Qt::LeftButton && retrieveCoordinateMode_m & 0x3E)
        {
            if(retrieveCoordinateMode_m == RectROI)
            {
                region_m.setBottomRight(getPointInImage(event));
                region_m = getAdjustedRegion();
                this->update();
                emit imageRectRegionSelected(region_m);
                retrieveCoordinateMode_m = DragROI;
                //qDebug() << region_m;
            }
            else if(retrieveCoordinateMode_m == DragROI)
            {
                this->setCursor(Qt::ArrowCursor);
                emit imageRectRegionSelected(getAdjustedRegion());
                //qDebug() << getAdjustedRegion();
            }
            else
            {
                emit imagePointSelected(getPointInImage(event));
            }
        }
    }

}

/* An override of mousePressEvent. If an image is attached and the left button is pressed
 * overtop of the ImageWidget while an image is present and the appropriate selectPixelMode_m
 * selected, the coordinates under the mouse are emitted as a QPoint relative to the
 * attachedImage_m's origin. If RectROI or DragROI is selected as the retrieveCoordinateMode_m
 * the bounds are checked of the ROI selected. If the mouseclick is outside of thouse bounds
 * a region will be drawn and enter RectROI with the starting ROI coordinates, else enter DragROI*/
void ImageLabel::mousePressEvent(QMouseEvent *event)
{
    //QLabel::mousePressEvent(event); @@@@ ?

    if(this->pixmap() != nullptr)
    {
        if(event->button() == Qt::LeftButton && retrieveCoordinateMode_m & 0x3D)
        {
            if(retrieveCoordinateMode_m == RectROI || retrieveCoordinateMode_m == DragROI)
            {
                dragStart_m = getPointInImage(event);

                //if point not within region_m, select and draw the ROI in RectROI mode.
                //else keep the starting point and move to DragROI mode shifting region_m
                if(dragStart_m.x() < region_m.topLeft().x()
                        || dragStart_m.y() < region_m.y()
                        || dragStart_m.x() > region_m.bottomRight().x()
                        || dragStart_m.y() > region_m.bottomRight().y())
                {
                    region_m.setTopLeft(dragStart_m);
                    region_m.setBottomRight(dragStart_m);
                    retrieveCoordinateMode_m = RectROI;
                }
                else
                {
                    this->setCursor(Qt::SizeAllCursor);
                    retrieveCoordinateMode_m = DragROI;
                }
            }
            else
            {
                emit imagePointSelected(getPointInImage(event));
            }
        }
    }

}

/* An override of mouseMoveEvent. If an image is attached and the mouse moves while the left
 * button is pressed overtop of the ImageWidget while an image present and the appropriate
 * selectPixelMode_m selected, the coordinates under the mouse are emitted as a QPoint relative
 * to the attachedImage_m's origin. If RectROI is the current coordinate mode, the bottom right
 * of the ROI is reassigned and the region drawn on the Pixmap. If DragROI, the region is shifted
 * by the drag distance and then redrawn on the pixmap*/
void ImageLabel::mouseMoveEvent(QMouseEvent *event)
{
    if(this->pixmap() != nullptr)
    {
        if(!event->pos().isNull() && retrieveCoordinateMode_m & 0x38)
        {
            if(retrieveCoordinateMode_m == RectROI)
            {
                region_m.setBottomRight(getPointInImage(event));
                this->update();
            }
            //this state's boundaries are checked in mouse press event, cant enter directly
            else if(retrieveCoordinateMode_m == DragROI)
            {
                QPoint endPoint = getPointInImage(event);
                QPoint dragDistance = dragStart_m - endPoint;
                dragStart_m = endPoint;
                region_m.setTopLeft(region_m.topLeft() - dragDistance);
                region_m.setBottomRight(region_m.bottomRight() - dragDistance);
                this->update();
            }
            else
            {
                emit imagePointSelected(getPointInImage(event));
            }
        }
    }
}

/* Override of paintEvent that paints 4 trapazoids around a ROI (region_m), if image is attached,
 * selected by the user through mouse events. Trapazoids are painted to darken the regions outside of the
 * selection as using QRegion would require listing an additional license if distrobuting on a linux machine.
 * ImageLabel_m is signaled to redraw after every paint occurs. */
void ImageLabel::paintEvent(QPaintEvent *event)
{
    QLabel::paintEvent(event);

    if(this->pixmap() != nullptr && retrieveCoordinateMode_m != NoClick)
    {
        QRect region = getAdjustedRegion(); //receiving unscaled region, must scale. use resize event to calculate scale and remove from get point in image
        QPainter painter(this);
        painter.setBrush(QColor(50, 50, 50));
        painter.setPen(QColor(50, 50, 50));
        painter.setCompositionMode(QPainter::CompositionMode_Darken);

        //draw region - 4 trapazoids

        //top
        QPoint polygon[4] = {
            this->rect().topLeft(),
            region.topLeft(),
            region.topRight(),
            this->rect().topRight()
        };
        painter.drawPolygon(polygon, 4);

        //bottom
        polygon[0] = this->rect().bottomLeft();
        polygon[1] = region.bottomLeft();
        polygon[2] = region.bottomRight();
        polygon[3] = this->rect().bottomRight();
        painter.drawPolygon(polygon, 4);

        //left
        polygon[0] = this->rect().topLeft();
        polygon[1] = region.topLeft();
        polygon[2] = region.bottomLeft();
        polygon[3] = this->rect().bottomLeft();
        painter.drawPolygon(polygon, 4);

        //right
        polygon[0] = this->rect().topRight();
        polygon[1] = region.topRight();
        polygon[2] = region.bottomRight();
        polygon[3] = this->rect().bottomRight();
        painter.drawPolygon(polygon, 4);
    }
}

//@@ COMMENTS
void ImageLabel::resizeEvent(QResizeEvent *event)
{
    QLabel::resizeEvent(event);

    if(this->pixmap() != nullptr && retrieveCoordinateMode_m != NoClick)
    {
       scaleWidth_m = this->pixmap()->width() / static_cast<float>(this->width());
       scaleHeight_m = this->pixmap()->height() / static_cast<float>(this->height());
    }

}

/* Translates the widget coordinates from the ImageWidget to the
 * imageLabel_m and scales the point to the appropriate position based on the image zoom.*/
QPoint ImageLabel::getPointInImage(QMouseEvent *event)
{
    QPoint mousePosition = event->pos();

    //x coordinate adjustment 
    mousePosition.setX(mousePosition.x() * scaleWidth_m);

    //y coordinate adjustment
    mousePosition.setY(mousePosition.y() * scaleHeight_m);

    //qDebug() << mousePosition;
    return mousePosition;
}

/* getAdjustedRegion uses the selected region_m member variable and cleans it up so that
 * it fits within the attachedImage_m's dimensions. It also reverts the region_m's top left
 * and bottom right corners if they become inverted due to mouse location during selection
 * so that the QPainter can draw the ROI if the ROI is selected from top left to bottom right
 * or vice versa.*/
QRect ImageLabel::getAdjustedRegion()
{
    int topLeftX, topLeftY, bottomRightX, bottomRightY;
    region_m.getCoords(&topLeftX, &topLeftY, &bottomRightX, &bottomRightY);

    if(topLeftX > bottomRightX) std::swap(topLeftX, bottomRightX);
    if(topLeftY > bottomRightY) std::swap(topLeftY, bottomRightY);
    if(topLeftX < 0) topLeftX = 0;
    if(topLeftY < 0) topLeftY = 0;
    if(bottomRightX >= this->pixmap()->width()) bottomRightX = this->pixmap()->width();
    if(bottomRightY >= this->pixmap()->height()) bottomRightY = this->pixmap()->height();

    //qDebug() << topLeftX << topLeftY << bottomRightX << bottomRightY;
    return QRect(QPoint(topLeftX, topLeftY), QPoint(bottomRightX, bottomRightY));
}

/* Member function setSelectPixelMode allows an external object to set the selectPixelMode_m member
 * variable which dictates how pixel locations are returned based on mouse action over an image.
 * The CoordinateMode enum represents the available modes and executed in the mouseEvent. */
void ImageLabel::setRetrieveCoordinateMode(uint mode)
{
    retrieveCoordinateMode_m = mode;
    if(mode == RectROI)
        initializePaintMembers();
}

/* A slot that when called, if region of interest mode is enabled, will pre-set the region in the image
 * and draw it on the Pixmap. Else it does nothing.*/
void ImageLabel::setRectRegionSelected(QRect roi)
{
    if(retrieveCoordinateMode_m == RectROI || retrieveCoordinateMode_m == DragROI)
    {
        region_m = roi;
        this->update();
    }
}

//Returns the current pixel selection status for cursor / displayed image interaction
uint ImageLabel::getRetrieveCoordinateMode()
{
    return retrieveCoordinateMode_m;
}

//initializes the member variables used for painting on the pixmap
void ImageLabel::initializePaintMembers()
{
    dragStart_m = QPoint(-1, -1);
    region_m = QRect(dragStart_m, dragStart_m);
    brushRadius_m = 0;
}
