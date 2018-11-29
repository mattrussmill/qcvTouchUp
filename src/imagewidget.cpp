/***********************************************************************
* FILENAME :    imagewidget.cpp
*
* LICENSE:
*       qcvTouchUp provides an image processing toolset for editing
*       photographs, purposed and packaged for use in a desktop application 
*       user environment. Copyright (C) 2018,  Matthew R. Miller
*
*       This program is free software: you can redistribute it and/or modify
*       it under the terms of the GNU General Public License as published by 
*       the Free Software Foundation (version 3 of the License) and the 
*       3-clause BSD License as agreed upon through the use of the Qt toolkit
*       and OpenCV libraries in qcvTouchUp development, respectively. Copies
*       of the appropriate license files for qcvTouchup, and its source code,
*       can be found in LICENSE.Qt.txt and LICENSE.CV.txt.
*
*       This program is distributed in the hope that it will be useful,
*       but WITHOUT ANY WARRANTY; without even the implied warranty of
*       MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*       GNU General Public License for more details.
*
*       You should have received a copy of the GNU General Public License and
*       3-clause BSD License along with this program.  If not, please see 
*       <http://www.gnu.org/licenses/> and <https://opencv.org/license.html>.
*    
*       If you wish to contact the developer about this project, please do so
*       through their account at <https://github.com/mattrussmill>
*
* DESCRIPTION :
*       This widget creates a scrollable area in which a QImage can be
*       viewed and resized without distortion. The area can also return
*       a selected point from within the image (scaled appropriately)
*       to be used in selecting colors within the displayed image.
*
* NOTES :
*       It is important to note that image widget does not maintain any attached
*       image buffers. It uses implicitly shared memory when handling QImages.
*
* AUTHOR :  Matthew R. Miller       START DATE :    January 7, 2018
*
* CHANGES : N/A - N/A
*
* VERSION       DATE            WHO                     DETAIL
* 0.1           01/22/2018      Matthew R. Miller       Initial Rev
* 0.2           06/23/2018      Matthew R. Miller       Drag and Drop Open
************************************************************************/

#include "imagewidget.h"
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QAction>
#include <QCursor>
#include <QPalette>
#include <QApplication>
#include <QDebug>
#include <QLabel>
#include <QScrollArea>
#include <QMenu>
#include <QPoint>
#include <QMutex>
#include <QUrl>
#include <QMimeData>
#include <QScrollBar>
#include <QRect>

#include <QDebug>

/* The ImageWidget constructor takes in one argument which is the parent QWidget
 * to handle desctuction at termination, else is set to nullptr by default. The
 * QWidget embeds a QLabel with an ignored size policy inside of a QScrollArea
 * so that the QLabel is unconstrained by a QLayout and does not skew during resizing.
 * Once the QScrollArea is embedded into the ImageWidget class, context menu actions
 * are set and appropriate signals are routed to perform those actions when triggered.*/
ImageWidget::ImageWidget(QWidget *parent) : QWidget(parent),
    imageLabel_m(new QLabel(this)), scrollArea_m(new QScrollArea(this))
{
    imageLabel_m->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    imageLabel_m->setScaledContents(true);
    imageLabel_m->setVisible(false);

    QPalette pal(QColor(0xa0, 0xa0, 0xa0));
    scrollArea_m->setPalette(pal);
    scrollArea_m->setAlignment(Qt::AlignCenter);
    scrollArea_m->setWidget(imageLabel_m);
    scrollArea_m->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea_m->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea_m->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    scrollArea_m->setVisible(true);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(scrollArea_m);
    layout->setMargin(0);
    setLayout(layout);
    setFocusPolicy(Qt::StrongFocus);

    setContextMenuPolicy(Qt::DefaultContextMenu);
    zoomInAction_m = new QAction("Zoom In", this);
    zoomOutAction_m = new QAction("Zoom Out", this);
    zoomFitAction_m = new QAction("Zoom Fill", this);
    zoomActualAction_m = new QAction("Zoom Actual", this);
    zoomInAction_m->setIconVisibleInMenu(false);
    zoomOutAction_m->setIconVisibleInMenu(false);
    zoomFitAction_m->setIconVisibleInMenu(false);
    zoomActualAction_m->setIconVisibleInMenu(false);
    connect(zoomInAction_m, SIGNAL(triggered()), this, SLOT(zoomIn()));
    connect(zoomOutAction_m, SIGNAL(triggered()), this, SLOT(zoomOut()));
    connect(zoomFitAction_m, SIGNAL(triggered()), this, SLOT(zoomFit()));
    connect(zoomActualAction_m, SIGNAL(triggered()), this, SLOT(zoomActual()));

    setAcceptDrops(true);
    mutex_m = nullptr;
}

//Member function which returns the current vertical scroll bar policy setting
Qt::ScrollBarPolicy ImageWidget::verticalScrollBarPolicy() const
{
    return scrollArea_m->verticalScrollBarPolicy();
}

//Member function which returns the current horizontal scroll bar policy setting
Qt::ScrollBarPolicy ImageWidget::horizontalScrollBarPolicy() const
{
    return scrollArea_m->horizontalScrollBarPolicy();
}

//Member function has one argument sbp, which is used to set the vertical ScrollBarPolicy
void ImageWidget::setVerticalScrollBarPolicy(Qt::ScrollBarPolicy sbp)
{
    scrollArea_m->setVerticalScrollBarPolicy(sbp);
}

//Member function has one argument sbp, which is used to set the horizontal ScrollBarPolicy
void ImageWidget::setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy sbp)
{
    scrollArea_m->setHorizontalScrollBarPolicy(sbp);
}

/* Slot overload of setImage attaches a QImage to be displayed through ImageWidget by pointing to
 * the memory location of the QImage. ImageWidget does not manage the attached QImage object.
 * Then setImage sets the image size to fill the ImageWidget container without distortion and
 * emits a signal to notify an image has been set. */
void ImageWidget::setImage(const QImage *image)
{
    if(image == nullptr)
    {
        emit imageNull();
        return;
    }

    //while waiting for mutex, process main event loop to keep gui responsive
    if(mutex_m)
    {
        while(!mutex_m->tryLock())
            QApplication::processEvents(QEventLoop::AllEvents, 100);
    }

    attachedImage_m = image;
    imageLabel_m->setPixmap(QPixmap::fromImage(*image));
    zoomFit();
    imageLabel_m->setVisible(true);
    if(mutex_m) mutex_m->unlock();
    emit imageSet();
}

/* Member function setFillWidget allows an external object to set the imageLabel_m containing the
 * displayed image to fill the ImageWidget without emitting a signal. (e.g. a QCheckBox signaled
 * by zoom* methods from ImageWidget which change its status when a zoom action occurs can set
 * this property without emitting a signal to itself).*/
void ImageWidget::setFillWidget(bool fill)
{
    if(fill) zoomFit();
    fillScrollArea_m = fill;
}

/* Member function setSelectPixelMode allows an external object to set the selectPixelMode_m member
 * variable which dictates how pixel locations are returned based on mouse action over an image.
 * The CoordinateMode enum represents the available modes and executed in the mouseEvent. */
void ImageWidget::setRetrieveCoordinateMode(CoordinateMode mode)
{
    retrieveCoordinateMode_m = mode;
}

//Returns the current pixel selection status for cursor / displayed image interaction
uint ImageWidget::getRetrieveCoordinateMode() const
{
    return retrieveCoordinateMode_m;
}

//Returns the current scale ratio between the displayed and attached image
double ImageWidget::currentScale() const
{
    return scalar_m;
}

//Returns true if an image is attached, false if not. Displays warning if debug.
bool ImageWidget::imageAttached() const
{
    if(!attachedImage_m || attachedImage_m->isNull())
    {
        qWarning("No QImage attached to ImageWidget!");
        return false;
    }
    return true;
}

//Returns if the ImageWidget fill property is enabled (true) or disabled (false).
bool ImageWidget::fillWidgetStatus() const
{
    return fillScrollArea_m;
}

//Returns the address of the currently displayed image buffer.
const QImage *ImageWidget::displayedImage()
{
    return attachedImage_m;
}

/*Sets and enables a mutex lock for updating the image from a QImage buffer.
 * Set to nullptr to disable*/
void ImageWidget::setMutex(QMutex &m)
{
    mutex_m = &m;
}

/* Member function clearImage clears the attached image pointer and hides the imageLabel_m containing
 * the QPixmap displaying the previously attached image. ImageWidget does not maintain the object
 * for the attached QImage data buffer and must be distroyed separately.*/
void ImageWidget::clearImage()
{
    if(attachedImage_m) attachedImage_m = nullptr;
    imageLabel_m->setVisible(false);
    emit imageCleared();
}

/* The zoomIn method scales the image up by 1/8. If the image attempts to be scaled
 * larget than a short signed int's largest value, the image does not increase in size
 * as that is the largest size QImage supports. If the image property 'fillScrollArea'
 * was set as true, it is set as false and a signal is emitted notifying as such.*/
void ImageWidget::zoomIn()
{
    if(!imageAttached()) return;
    float tmpScalar = scalar_m * 1.125;
    if (tmpScalar * imageLabel_m->width() > 32768 ||
            tmpScalar * imageLabel_m->height() > 32768)
    {
        qWarning("ImageWidget displaying QImage at maximum size!");
        return;
    }
    scalar_m = tmpScalar;
    imageLabel_m->resize(scalar_m * attachedImage_m->size());
    if (fillScrollArea_m == true)
    {
        fillScrollArea_m = false;
        emit fillWidgetChanged(false);
    }
}

/* The zoomIn member scales the image down by about 1/8. If the image property 'fillScrollArea'
 * was set as true, it is set as false and a signal is emitted notifying as such.*/
void ImageWidget::zoomOut()
{
    if(!imageAttached()) return;
    scalar_m *= 0.889;
    imageLabel_m->resize(scalar_m * attachedImage_m->size());
    if (fillScrollArea_m == true)
    {
        fillScrollArea_m = false;
        emit fillWidgetChanged(false);
    }
}

/* The zoomFit member function scales the image to fit full size within the ImageWidget while
 * maintaining its aspect ratio. It does so by scaling by the smaller aspect ratio relation
 * between width and height. If the fill property is false, its set to true and emits a signal*/
void ImageWidget::zoomFit()
{
    if(!imageAttached()) return;
    float widthRatio = scrollArea_m->width() / static_cast<float>(attachedImage_m->width());
    float heightRatio = scrollArea_m->height() / static_cast<float>(attachedImage_m->height());
    if(widthRatio > heightRatio)
        scalar_m = heightRatio;
    else
        scalar_m = widthRatio;
    imageLabel_m->resize(scalar_m * attachedImage_m->size() - QSize(2, 2));
    if (fillScrollArea_m == false)
    {
        fillScrollArea_m = true;
        emit fillWidgetChanged(true);
    }
}

/* The zoomActual member function scales the image to the actual dimension of the attached
 * QImage. If the image property 'fillScrollArea'
 * was set as true, it is set as false and a signal is emitted notifying as such.*/
void ImageWidget::zoomActual()
{
    if(!imageAttached()) return;
    imageLabel_m->adjustSize();
    scalar_m = 1.0;
    if (fillScrollArea_m == true)
    {
        fillScrollArea_m = false;
        emit fillWidgetChanged(false);
    }
}

/*When called, the Pixmap is refreshed (reloaded) with the attached QImage but not resized.
 * Because of the possibility this function will operate on an image outside of the class,
 * a mutex locks the operation if it is available*/
void ImageWidget::updateDisplayedImage()
{
    if(!imageAttached()) return;

    //while waiting for mutex, process main event loop to keep gui responsive
    if(mutex_m)
    {
        while(!mutex_m->tryLock())
            QApplication::processEvents(QEventLoop::AllEvents, 100);
    }

    imageLabel_m->setPixmap(QPixmap::fromImage(*attachedImage_m));
    if(mutex_m) mutex_m->unlock();
}

/* Overload of updateDisplayedImage. Attaches a new image buffer without resizing. Because
 * of the possibility this function will operate on an image outside of the class, a mutex
 * locks the operation if it is available*/
void ImageWidget::updateDisplayedImage(const QImage *image)
{
    if(image == nullptr) return;

    //while waiting for mutex, process main event loop to keep gui responsive
    if(mutex_m)
    {
        while(!mutex_m->tryLock())
            QApplication::processEvents(QEventLoop::AllEvents, 100);
    }

    imageLabel_m->setPixmap(QPixmap::fromImage(*image));
    attachedImage_m = image;
    if(mutex_m) mutex_m->unlock();
}

/* An override of resizeEvent. When ImageWidget is resized if 'fillScrollArea' property is true
 * the imageLabel_m is scaled to fill the ImageWidget. Function is virtural so that it may be
 * overridden if ImageWidget is expanded upon in another object. The original resizeEvent is
 * then called so that other resize operations standard to QWidgets can occur.*/
void ImageWidget::resizeEvent(QResizeEvent *event)
{
    if(imageAttached())
        if(fillScrollArea_m) zoomFit();
    QWidget::resizeEvent(event);
}

/* An override of mouseReleaseEvent. If an image is attached and the left button is released
 * overtop of the ImageWidget while an image is present and the appropriate selectPixelMode_m
 * selected, the coordinates under the mouse are emitted as a QPoint relative to the
 * attachedImage_m's origin.*/
void ImageWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if(imageAttached())
    {
        if(event->button() == Qt::LeftButton && (retrieveCoordinateMode_m == SingleUnclick
                                                 || retrieveCoordinateMode_m == ClickUnclick
                                                 || retrieveCoordinateMode_m == ClickDrag
                                                 || retrieveCoordinateMode_m == RectROI))
        {
            if(retrieveCoordinateMode_m == RectROI)
            {
                region_m.setBottomRight(getPointInImage());
                //paint event
                emit imageRectRegionSelected(region_m);
                qDebug() << region_m;
            }
            else
            {
                emit imagePointSelected(getPointInImage());
            }
        }
    }
}

/* An override of mousePressEvent. If an image is attached and the left button is pressed
 * overtop of the ImageWidget while an image is present and the appropriate selectPixelMode_m
 * selected, the coordinates under the mouse are emitted as a QPoint relative to the
 * attachedImage_m's origin.*/
void ImageWidget::mousePressEvent(QMouseEvent *event)
{
    if(imageAttached())
    {
        if(event->button() == Qt::LeftButton && (retrieveCoordinateMode_m == SingleClick
                                                 ||retrieveCoordinateMode_m == ClickUnclick
                                                 || retrieveCoordinateMode_m == ClickDrag
                                                 || retrieveCoordinateMode_m == RectROI))
        {
            if(retrieveCoordinateMode_m == RectROI)
                region_m.setTopLeft(getPointInImage());
            else
                emit imagePointSelected(getPointInImage());
        }
    }
}

/* An override of mouseMoveEvent. If an image is attached and the mouse moves while the left
 * button is pressed overtop of the ImageWidget while an image present and the appropriate
 * selectPixelMode_m selected, the coordinates under the mouse are emitted as a QPoint relative
 * to the attachedImage_m's origin.*/
void ImageWidget::mouseMoveEvent(QMouseEvent *event)
{
    if(imageAttached())
    {
        if(!event->pos().isNull() == Qt::LeftButton && (retrieveCoordinateMode_m == ClickDrag
                                                        || retrieveCoordinateMode_m == RectROI))
        {
            if(retrieveCoordinateMode_m == RectROI)
            {
                region_m.setBottomRight(getPointInImage());
                //paint event
            }
            else
                emit imagePointSelected(getPointInImage());
        }
    }
}

/* An override of contextMenuEvent. When the ImageWidget's context menu button is activated
 * (default right click) a menu is generated at that location to access the zoom actions for
 * the imageLabel_m if a QImage is attached. Function is virtural so that it may be overridden
 * if ImageWidget is expanded upon in another object.*/
void ImageWidget::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *zoomMenu = new QMenu(this);
    zoomMenu->addAction(zoomInAction_m);
    zoomMenu->addAction(zoomOutAction_m);
    zoomMenu->addAction(zoomFitAction_m);
    zoomMenu->addAction(zoomActualAction_m);
    zoomMenu->exec(event->globalPos());
    delete zoomMenu;
}

/* An override of dragEnterEvent. When an object is dragged over top of this widget, the event
 * checks to see if the object has a valid path. If the path is valid, the ImageWidget background
 * is lightened to indicate the widget will accept the drop and accepts it, else it ignores the
 * drop and stops.*/
void ImageWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if(event->mimeData()->hasUrls())
    {
        QPalette pal(QColor(0xaf, 0xaf, 0xaf));
        scrollArea_m->setPalette(pal);
        event->setAccepted(true);
        event->acceptProposedAction();
    }
    else
    {
        event->setAccepted(false);
    }

}

/* An override of dragLeaveEvent. If dragEnterEvent marks the dragged object over ImageWidget as
 * "accepted" this event changes the background color back to its initial color upon exiting the
 * widget.*/
void ImageWidget::dragLeaveEvent(QDragLeaveEvent *event)
{
    if(event->isAccepted())
    {
        QPalette pal(QColor(0xa0, 0xa0, 0xa0));
        scrollArea_m->setPalette(pal);
    }
}

/* An override of dropEvent. If dragEnterEvent "accepts" the event, dropEvent checks to make sure
 * only 1 file was dropped in the ImageWidget area. If more than one was dropped an error is
 * emitted. If only one file is dropped, the background color is reset to its initial color and
 * the QString containing the image path emitted.*/
void ImageWidget::dropEvent(QDropEvent *event)
{
    if(event->mimeData()->urls().size() == 1)
    {
        QPalette pal(QColor(0xa0, 0xa0, 0xa0));
        scrollArea_m->setPalette(pal);
        QString filePath = event->mimeData()->urls().at(0).toLocalFile();
        emit droppedImagePath(filePath);
    }
    else
    {
        qDebug() << "Too many files dragged onto ImageWidget";
        emit droppedImageError();
    }
}

/* If imageLabel is the same size or smaller, shift the origin of the ImageWidget to the origin of the
 * imageLabel. Else shift the origin based on the scrollBar positions using the image scrollArea as
 * the ROI window. Adjusts for scrollBar space as well. QLabel could have been overloaded to avoid this
 * hell of compensating for image position within the scroll area.*/
QPoint ImageWidget::getPointInImage()
{
    if(imageAttached())
    {
        QPoint mousePosition = mapFromGlobal(QCursor::pos());

        //x coordinate adjustment
        float scalar =  attachedImage_m->width() / static_cast<float>(imageLabel_m->width());
        if(imageLabel_m->width() <= this->width())
        {
            mousePosition += QPoint((imageLabel_m->width() - scrollArea_m->width()) / 2, 0);
        }
        else
        {
            float scrollBarPosition = scrollArea_m->horizontalScrollBar()->value()
                    / static_cast<float>(scrollArea_m->horizontalScrollBar()->maximum());

            mousePosition += QPoint(scrollBarPosition * (imageLabel_m->width() - scrollArea_m->width()
                                                         + scrollArea_m->verticalScrollBar()->width() + 1), 0);
        }
        mousePosition.setX(mousePosition.x() * scalar);

        //y coordinate adjustment
        scalar =  attachedImage_m->height() / static_cast<float>(imageLabel_m->height());
        if(imageLabel_m->height() <= this->height())
        {
            mousePosition += QPoint(0, (imageLabel_m->height() - scrollArea_m->height()) / 2);
        }
        else
        {
            float scrollBarPosition = scrollArea_m->verticalScrollBar()->value()
                    / static_cast<float>(scrollArea_m->verticalScrollBar()->maximum());

            mousePosition += QPoint(0, scrollBarPosition * (imageLabel_m->height() - scrollArea_m->height()
                                                            + scrollArea_m->horizontalScrollBar()->height() + 1));
        }
        mousePosition.setY(mousePosition.y() * scalar);

        qDebug() << mousePosition;
        return mousePosition;
    }
    return QPoint();
}


