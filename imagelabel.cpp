/***********************************************************************
* FILENAME :    imagelabel.h
*
* LICENSE:
*       qcvTouchUp provides an image processing toolset for editing
*       photographs, purposed and packaged for use in a desktop application
*       user environment. Copyright (C) 2018,  Matthew R. Miller
*
*       This program is free software: you can redistribute it and/or modify
*       it under the terms of the GNU General Public License as published by
*       the Free Software Foundation (version 3 of the License).
*
*       The framework and libraries used to create this software are licenced
*       under the  GNU Lesser General Public License (LGPL) version 3 and the
*       3-clause BSD License as agreed upon through the use of the Qt toolkit
*       and OpenCV libraries respectively. Copies of the appropriate licenses
*       for qcvTouchup, and its source code, can be found in LICENSE.txt,
*       LICENSE.Qt.txt, and LICENSE.CV.txt. If not, please see
*       <http://www.gnu.org/licenses/> and <https://opencv.org/license.html>
*       for further information on licensing.
*
*       This program is distributed in the hope that it will be useful,
*       but WITHOUT ANY WARRANTY; without even the implied warranty of
*       MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*       GNU General Public License for more details.
*
*       If you wish to contact the developer about this project, please do so
*       through their account at <https://github.com/mattrussmill>
*
* DESCRIPTION :
*       This class extends the QLabel class to display the crop region
*       visualizations necessary to use the TransformMenu object effectively.
*
* NOTES :
*
*
* AUTHOR :  Matthew R. Miller       START DATE :    March 03/04/2019
*
* CHANGES : N/A - N/A
*
* VERSION       DATE            WHO                     DETAIL
* 0.1           10/20/2019      Matthew R. Miller       Initial Rev
*
************************************************************************/
#include "imagelabel.h"
#include <QMouseEvent>
#include <QPainter>
#include <QDebug>

//constructor
ImageLabel::ImageLabel(QWidget *parent) : QLabel(parent)
{
    setObjectName("imageWidgetBackground");
    initializePaintMembers();
}

/* An override of mouseReleaseEvent. If an image is attached and the left button is released
 * overtop of the ImageWidget while an image is present and the appropriate selectPixelMode_m
 * selected, the coordinates under the mouse are emitted as a QPoint relative to the
 * attachedImage_m's origin. If RectROI is selected, the region is finished being set, cropped
 * and drawn before being emitted, else if DragROI the region is only cropped to fit and emitted. */
void ImageLabel::mouseReleaseEvent(QMouseEvent *event)
{
    QLabel::mouseReleaseEvent(event);

    if(this->pixmap() != nullptr)
    {
        if(event->button() == Qt::LeftButton && retrieveCoordinateMode_m & 0x3E)
        {
            if(retrieveCoordinateMode_m == RectROI)
            {
                //set scaled to image
                imageRegion_m.setBottomRight(getPointInImage(event));
                imageRegion_m = getAdjustedImageRegion();

                //set to widget
                widgetRegion_m.setBottomRight(event->pos());
                widgetRegion_m = getAdjustedWidgetRegion();

                this->update();
                emit imageRectRegionSelected(imageRegion_m);
                retrieveCoordinateMode_m = DragROI;
                qDebug() << imageRegion_m;
            }
            else if(retrieveCoordinateMode_m == DragROI)
            {
                this->setCursor(Qt::ArrowCursor);
                emit imageRectRegionSelected(getAdjustedImageRegion());
                qDebug() << getAdjustedImageRegion();
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
    QLabel::mousePressEvent(event);

    if(this->pixmap() != nullptr)
    {
        if(event->button() == Qt::LeftButton && retrieveCoordinateMode_m & 0x3D)
        {
            startPainting = true;
            if(retrieveCoordinateMode_m == RectROI || retrieveCoordinateMode_m == DragROI)
            {
                imageDragStart_m = getPointInImage(event);
                widgetDragStart_m = event->pos();

                //if point not within widgetRegion_m, select and draw the ROI in RectROI mode.
                //else keep the starting point and move to DragROI mode shifting imageRegion_m
                if(widgetDragStart_m.x() < widgetRegion_m.topLeft().x()
                        || widgetDragStart_m.y() < widgetRegion_m.y()
                        || widgetDragStart_m.x() > widgetRegion_m.bottomRight().x()
                        || widgetDragStart_m.y() > widgetRegion_m.bottomRight().y())
                {
                    //set scaled to image
                    imageRegion_m.setTopLeft(imageDragStart_m);
                    imageRegion_m.setBottomRight(imageDragStart_m);

                    //set to widget
                    widgetRegion_m.setTopLeft(widgetDragStart_m);
                    widgetRegion_m.setBottomRight(widgetDragStart_m);

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
    QLabel::mouseMoveEvent(event);

    if(this->pixmap() != nullptr)
    {
        if(!event->pos().isNull() && retrieveCoordinateMode_m & 0x38)
        {
            if(retrieveCoordinateMode_m == RectROI)
            {
                //set scaled to image
                imageRegion_m.setBottomRight(getPointInImage(event));

                //set to widget
                widgetRegion_m.setBottomRight(event->pos());

                this->update();
            }
            //this state's boundaries are checked in mouse press event, cant enter directly
            else if(retrieveCoordinateMode_m == DragROI)
            {
                //set scaled to image
                QPoint endPoint = getPointInImage(event);
                QPoint dragDistance = imageDragStart_m - endPoint;
                imageDragStart_m = endPoint;
                imageRegion_m.setTopLeft(imageRegion_m.topLeft() - dragDistance);
                imageRegion_m.setBottomRight(imageRegion_m.bottomRight() - dragDistance);

                //set to widget
                endPoint = event->pos();
                dragDistance = widgetDragStart_m - endPoint;
                widgetDragStart_m = endPoint;
                widgetRegion_m.setTopLeft(widgetRegion_m.topLeft() - dragDistance);
                widgetRegion_m.setBottomRight(widgetRegion_m.bottomRight() - dragDistance);

                this->update();
            }
            else
            {
                emit imagePointSelected(getPointInImage(event));
            }
        }
    }
}

/* Override of paintEvent that paints 4 trapazoids around a ROI (imageRegion_m), if image is attached,
 * selected by the user through mouse events. Trapazoids are painted to darken the regions outside of the
 * selection as using QRegion would require listing an additional license if distrobuting on a linux machine.
 * ImageLabel_m is signaled to redraw after every paint occurs. */
void ImageLabel::paintEvent(QPaintEvent *event)
{
    QLabel::paintEvent(event);

    if(startPainting && this->pixmap() != nullptr && retrieveCoordinateMode_m != NoClick)
    {
        QRect region = getAdjustedWidgetRegion();
        QPainter painter(this);
        painter.setBrush(QColor(50, 50, 50));
        painter.setPen(QColor(50, 50, 50));
        painter.setCompositionMode(QPainter::CompositionMode_Darken);

        //draw region - 4 trapazoids //print out

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

/* Translates the widget coordinates from the ImageWidget to the
 * imageLabel_m and scales the point to the appropriate position based on the image zoom.*/
QPoint ImageLabel::getPointInImage(QMouseEvent *event)
{
    QPoint mousePosition = event->pos();

    //x coordinate adjustment 
    mousePosition.setX(mousePosition.x() * //scale width
                       (this->pixmap()->width() / static_cast<float>(this->width())));

    //y coordinate adjustment
    mousePosition.setY(mousePosition.y() * //scale height
                       (this->pixmap()->height() / static_cast<float>(this->height())));

    //qDebug() << mousePosition;
    return mousePosition;
}

/* getAdjustedImageRegion uses the selected imageRegion_m member variable and cleans it up so that
 * it fits within the attachedImage_m's dimensions. It also reverts the imageRegion_m's top left
 * and bottom right corners if they become inverted due to mouse location during selection
 * so that the QPainter can draw the ROI if the ROI is selected from top left to bottom right
 * or vice versa.*/
QRect ImageLabel::getAdjustedImageRegion()
{
    int topLeftX, topLeftY, bottomRightX, bottomRightY;
    imageRegion_m.getCoords(&topLeftX, &topLeftY, &bottomRightX, &bottomRightY);

    if(topLeftX > bottomRightX) std::swap(topLeftX, bottomRightX);
    if(topLeftY > bottomRightY) std::swap(topLeftY, bottomRightY);
    if(topLeftX < 0) topLeftX = 0;
    if(topLeftY < 0) topLeftY = 0;
    if(bottomRightX >= this->pixmap()->width()) bottomRightX = this->pixmap()->width();
    if(bottomRightY >= this->pixmap()->height()) bottomRightY = this->pixmap()->height();

    //qDebug() << topLeftX << topLeftY << bottomRightX << bottomRightY;
    return QRect(QPoint(topLeftX, topLeftY), QPoint(bottomRightX, bottomRightY));
}


/* getAdjustedWidgetRegion uses the selected widgetRegion_m member variable and cleans it up so that
 * it fits within the widget's dimensions. It also reverts the imageRegion_m's top left
 * and bottom right corners if they become inverted due to mouse location during selection
 * so that the QPainter can draw the ROI if the ROI is selected from top left to bottom right
 * or vice versa.*/
QRect ImageLabel::getAdjustedWidgetRegion()
{
    int topLeftX, topLeftY, bottomRightX, bottomRightY;
    widgetRegion_m.getCoords(&topLeftX, &topLeftY, &bottomRightX, &bottomRightY);

    if(topLeftX > bottomRightX) std::swap(topLeftX, bottomRightX);
    if(topLeftY > bottomRightY) std::swap(topLeftY, bottomRightY);
    if(topLeftX < 0) topLeftX = 0;
    if(topLeftY < 0) topLeftY = 0;
    if(bottomRightX >= this->width()) bottomRightX = this->width();
    if(bottomRightY >= this->height()) bottomRightY = this->height();

    qDebug() << topLeftX << topLeftY << bottomRightX << bottomRightY;
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
        //set image region
        imageRegion_m = roi;

        //scale for widget region
        //(static_cast<float>(this->width())) / this->pixmap()->width())
        roi.setTopLeft(QPoint(roi.topLeft().x() * ((static_cast<float>(this->width())) / this->pixmap()->width()),
                              roi.topLeft().y() * ((static_cast<float>(this->height())) / this->pixmap()->height())));

        roi.setBottomRight(QPoint(roi.bottomRight().x() * ((static_cast<float>(this->width())) / this->pixmap()->width()),
                                  roi.bottomRight().y() * ((static_cast<float>(this->height())) / this->pixmap()->height())));
        widgetRegion_m = roi;

        startPainting = true;
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
    startPainting = false;
    imageDragStart_m = QPoint(-1, -1);
    widgetDragStart_m = imageDragStart_m;
    imageRegion_m = QRect(imageDragStart_m, imageDragStart_m);
    widgetRegion_m = QRect(widgetDragStart_m, widgetDragStart_m);
    brushRadius_m = 0;
}
