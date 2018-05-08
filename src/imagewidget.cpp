/***********************************************************************
* FILENAME :    quickmenu.cpp
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
*
************************************************************************/

#include "imagewidget.h"
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QAction>
#include <QCursor>
#include <QPalette>
#include <QApplication>
#include <QDebug>

/* The ImageWidget constructor takes in one argument which is the parent QWidget
 * to handle desctuction at termination, else is set to nullptr by default. The
 * QWidget embeds a QLabel with an ignored size policy inside of a QScrollArea
 * so that the QLabel is unconstrained by a QLayout and does not skew during resizing.
 * Once the QScrollArea is embedded into the ImageWidget class, context menu actions
 * are set and appropriate signals are routed to perform those actions when triggered.*/
ImageWidget::ImageWidget(QWidget *parent) : QWidget(parent),
    imageLabel(new QLabel(this)), scrollArea(new QScrollArea(this))
{
    imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    imageLabel->setScaledContents(true);
    imageLabel->setVisible(false);

    QPalette pal(QColor(0xa0, 0xa0, 0xa0));
    scrollArea->setPalette(pal);
    scrollArea->setWidget(imageLabel);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    scrollArea->setVisible(true);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(scrollArea);
    layout->setMargin(0);
    setLayout(layout);
    setFocusPolicy(Qt::StrongFocus);

    setContextMenuPolicy(Qt::DefaultContextMenu);
    zoomInAction = new QAction("Zoom In", this);
    zoomOutAction = new QAction("Zoom Out", this);
    zoomFitAction = new QAction("Zoom Fill", this);
    zoomActualAction = new QAction("Zoom Actual", this);
    zoomInAction->setIconVisibleInMenu(false);
    zoomOutAction->setIconVisibleInMenu(false);
    zoomFitAction->setIconVisibleInMenu(false);
    zoomActualAction->setIconVisibleInMenu(false);
    connect(zoomInAction, SIGNAL(triggered()), this, SLOT(zoomIn()));
    connect(zoomOutAction, SIGNAL(triggered()), this, SLOT(zoomOut()));
    connect(zoomFitAction, SIGNAL(triggered()), this, SLOT(zoomFit()));
    connect(zoomActualAction, SIGNAL(triggered()), this, SLOT(zoomActual()));
    mutex = nullptr;
}

//Member function which returns the last selected point of a loaded image
QPoint ImageWidget::lastPointSelected() const
{
    return selectedPoint;
}

//Member function which returns the current vertical scroll bar policy setting
Qt::ScrollBarPolicy ImageWidget::verticalScrollBarPolicy() const
{
    return scrollArea->verticalScrollBarPolicy();
}

//Member function which returns the current horizontal scroll bar policy setting
Qt::ScrollBarPolicy ImageWidget::horizontalScrollBarPolicy() const
{
    return scrollArea->horizontalScrollBarPolicy();
}

//Member function has one argument sbp, which is used to set the vertical ScrollBarPolicy
void ImageWidget::setVerticalScrollBarPolicy(Qt::ScrollBarPolicy sbp)
{
    scrollArea->setVerticalScrollBarPolicy(sbp);
}

//Member function has one argument sbp, which is used to set the horizontal ScrollBarPolicy
void ImageWidget::setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy sbp)
{
    scrollArea->setHorizontalScrollBarPolicy(sbp);
}

/* Member function setImage attaches a QImage to be displayed through ImageWidget by pointing to
 * the memory location of the QImage. ImageWidget does not manage the attached QImage object.
 * Then setImage sets the image size to fill the ImageWidget container without distortion and
 * emits a signal to notify an image has been set. */
void ImageWidget::setImage(const QImage &image)
{
    if(image.isNull())
    {
        emit imageNull();
        return;
    }

    //while waiting for mutex, process main event loop to keep gui responsive
    if(mutex)
    {
        while(!mutex->tryLock())
            QApplication::processEvents(QEventLoop::AllEvents, 100);
    }

    attachedImage = &image;
    imageLabel->setPixmap(QPixmap::fromImage(image));
    zoomFit();
    imageLabel->setVisible(true);
    if(mutex) mutex->unlock();
    emit imageSet();
}

/* Member function setFillWidget allows an external object to set the imageLabel containing the
 * displayed image to fill the ImageWidget without emitting a signal. (e.g. a QCheckBox signaled
 * by zoom* methods from ImageWidget which change its status when a zoom action occurs can set
 * this property without emitting a signal to itself).*/
void ImageWidget::setFillWidget(bool fill)
{
    if(fill) zoomFit();
    fillScrollArea = fill;
}

//Returns the current scale ratio between the displayed and attached image
double ImageWidget::currentScale() const
{
    return scalar;
}

//Returns true if an image is attached, false if not. Displays warning if debug.
bool ImageWidget::imageAttached() const
{
    if(!attachedImage || attachedImage->isNull())
    {
        qWarning("No QImage attached to ImageWidget!");
        return false;
    }
    return true;
}

//Returns if the ImageWidget fill property is enabled (true) or disabled (false).
bool ImageWidget::fillWidgetStatus() const
{
    return fillScrollArea;
}

//Returns the address of the currently displayed image buffer.
const QImage *ImageWidget::displayedImage()
{
    return attachedImage;
}

/*Sets and enables a mutex lock for updating the image from a QImage buffer.
 * Set to nullptr to disable*/
void ImageWidget::setMutex(QMutex &m)
{
    mutex = &m;
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
    if(mutex)
    {
        while(!mutex->tryLock())
            QApplication::processEvents(QEventLoop::AllEvents, 100);
    }

    attachedImage = image;
    imageLabel->setPixmap(QPixmap::fromImage(*image));
    zoomFit();
    imageLabel->setVisible(true);
    if(mutex) mutex->unlock();
    emit imageSet();

}

/* Member function clearImage clears the attached image pointer and hides the imageLabel containing
 * the QPixmap displaying the previously attached image. ImageWidget does not maintain the object
 * for the attached QImage data buffer and must be distroyed separately.*/
void ImageWidget::clearImage()
{
    if(attachedImage) attachedImage = nullptr;
    imageLabel->setVisible(false);
    emit imageCleared();
}

/* The zoomIn method scales the image up by 1/8. If the image attempts to be scaled
 * larget than a short signed int's largest value, the image does not increase in size
 * as that is the largest size QImage supports. If the image property 'fillScrollArea'
 * was set as true, it is set as false and a signal is emitted notifying as such.*/
void ImageWidget::zoomIn()
{
    if(!imageAttached()) return;
    float tmpScalar = scalar * 1.125;
    if (tmpScalar * imageLabel->width() > 32768 ||
            tmpScalar * imageLabel->height() > 32768)
    {
        qWarning("ImageWidget displaying QImage at maximum size!");
        return;
    }
    scalar = tmpScalar;
    imageLabel->resize(scalar * attachedImage->size());
    if (fillScrollArea == true)
    {
        fillScrollArea = false;
        emit fillWidgetChanged(false);
    }
}

/* The zoomIn member scales the image down by about 1/8. If the image property 'fillScrollArea'
 * was set as true, it is set as false and a signal is emitted notifying as such.*/
void ImageWidget::zoomOut()
{
    if(!imageAttached()) return;
    scalar *= 0.889;
    imageLabel->resize(scalar * attachedImage->size());
    if (fillScrollArea == true)
    {
        fillScrollArea = false;
        emit fillWidgetChanged(false);
    }
}

/* The zoomFit member function scales the image to fit full size within the ImageWidget while
 * maintaining its aspect ratio. It does so by scaling by the smaller aspect ratio relation
 * between width and height. If the fill property is false, its set to true and emits a signal*/
void ImageWidget::zoomFit()
{
    if(!imageAttached()) return;
    float widthRatio = scrollArea->width() / static_cast<float>(attachedImage->width());
    float heightRatio = scrollArea->height() / static_cast<float>(attachedImage->height());
    if(widthRatio > heightRatio)
        scalar = heightRatio;
    else
        scalar = widthRatio;
    imageLabel->resize(scalar * attachedImage->size() - QSize(2, 2));
    if (fillScrollArea == false)
    {
        fillScrollArea = true;
        emit fillWidgetChanged(true);
    }
}

/* The zoomActual member function scales the image to the actual dimension of the attached
 * QImage. If the image property 'fillScrollArea'
 * was set as true, it is set as false and a signal is emitted notifying as such.*/
void ImageWidget::zoomActual()
{
    if(!imageAttached()) return;
    imageLabel->adjustSize();
    scalar = 1.0;
    if (fillScrollArea == true)
    {
        fillScrollArea = false;
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
    if(mutex)
    {
        while(!mutex->tryLock())
            QApplication::processEvents(QEventLoop::AllEvents, 100);
    }

    imageLabel->setPixmap(QPixmap::fromImage(*attachedImage));
    if(mutex) mutex->unlock();
}

/*Overload of updateDisplayedImage. Attaches a new image buffer without resizing. Because
 * of the possibility this function will operate on an image outside of the class, a mutex
 * locks the operation if it is available*/
void ImageWidget::updateDisplayedImage(const QImage *image)
{
    if(image == nullptr) return;

    //while waiting for mutex, process main event loop to keep gui responsive
    if(mutex)
    {
        while(!mutex->tryLock())
            QApplication::processEvents(QEventLoop::AllEvents, 100);
    }

    imageLabel->setPixmap(QPixmap::fromImage(*image));
    attachedImage = image;
    if(mutex) mutex->unlock();
}

/* An override of resizeEvent. When ImageWidget is resized if 'fillScrollArea' property is true
 * the imageLabel is scaled to fill the ImageWidget. Function is virtural so that it may be
 * overridden if ImageWidget is expanded upon in another object. The original resizeEvent is
 * then called so that other resize operations standard to QWidgets can occur.*/
void ImageWidget::resizeEvent(QResizeEvent *event)
{
    if(imageAttached())
        if(fillScrollArea) zoomFit();
    QWidget::resizeEvent(event);
}

/* An override of mouseReleaseEvent. If an image is attached and the left button is released
 * overtop of the imageLabel (bounds checked), the coordinates are recorded of the pixel under
 * the cursor when the button is released. That pixel coordinate is then scaled to map to the
 * attached QImage's actual pixel coordinates.If the scaled pixel is out of bounds, it is
 * adjusted to fit within the QImage coordinates. Function is virtural so that it may be
 * overridden if ImageWidget is expanded upon in another object.*/
void ImageWidget::mouseReleaseEvent(QMouseEvent *event)
{
    int x1, y1, x2, y2;
    QPoint mousePosition;
    if(imageAttached())
    {
        if(event->button() == Qt::LeftButton)
        {
            imageLabel->rect().getCoords(&x1, &y1, &x2, &y2);
            mousePosition = mapFromGlobal(QCursor::pos());
            if(mousePosition.x() > x2 || mousePosition.y() > y2) return;
            mousePosition *= 1/scalar;
            if(mousePosition.x() > attachedImage->width() - 1)
                mousePosition.setX(attachedImage->width() - 1);
            if(mousePosition.y() > attachedImage->height() - 1)
                mousePosition.setY(attachedImage->height() - 1);
            selectedPoint = mousePosition;
            emit imagePointSelected(selectedPoint);
        }
    }
}

/* An override of contextMenuEvent. When the ImageWidget's context menu button is activated
 * (default right click) a menu is generated at that location to access the zoom actions for
 * the imageLabel if a QImage is attached. Function is virtural so that it may be overridden
 * if ImageWidget is expanded upon in another object.*/
void ImageWidget::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *zoomMenu = new QMenu(this);
    zoomMenu->addAction(zoomInAction);
    zoomMenu->addAction(zoomOutAction);
    zoomMenu->addAction(zoomFitAction);
    zoomMenu->addAction(zoomActualAction);
    zoomMenu->exec(event->globalPos());
    delete zoomMenu;
}


