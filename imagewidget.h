/***********************************************************************
* FILENAME :    imagewidget.h
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
*       This widget creates a scrollable area in which a QImage can be
*       viewed and resized without distortion. The area can also return
*       a selected point from within the image (scaled appropriately)
*       to be used in selecting colors within the displayed image.
*
* NOTES :
*       QWidget was subclassed to ImageWidget instead of QScrollArea so
*       that most of the QScrollArea members remain encapsulated and
*       private.
*
* AUTHOR :  Matthew R. Miller       START DATE :    January 7, 2018
*
* CHANGES : N/A - N/A
*
* VERSION       DATE            WHO                     DETAIL
* 0.1           01/22/2018      Matthew R. Miller       Initial Rev
* 0.2           06/23/2018      Matthew R. Miller       Drag and Drop Open
* 0.3           10/26/2018      Matthew R. Miller       Pixel Selection Added
* 0.4           12/17/2018      Matthew R. Miller       ROI Selection Added
************************************************************************/

#ifndef IMAGEWIDGET_H
#define IMAGEWIDGET_H

#include <QWidget>
#include <opencv2/core/core.hpp>
class ImageLabel;
class QScrollArea;
class QImage;
class QMenu;
class QMutex;
class QPixmap;
class MouseWheelCtrlEaterEventFilter;

class ImageWidget : public QWidget
{
    Q_OBJECT

public:
    ImageWidget(QWidget *parent = nullptr);
    Qt::ScrollBarPolicy verticalScrollBarPolicy() const;
    Qt::ScrollBarPolicy horizontalScrollBarPolicy() const;
    void setVerticalScrollBarPolicy(Qt::ScrollBarPolicy sbp = Qt::ScrollBarAsNeeded);
    void setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy sbp = Qt::ScrollBarAsNeeded);
    void setFillWidget(bool fill = true);
    uint getRetrieveCoordinateMode() const; //change to RetrieveCoordinateMode
    float currentScale() const;
    bool imageAttached() const;
    bool fillWidgetStatus() const;
    const QImage* displayedImage();
    ImageLabel *imageLabel_m;
    void setMutex(QMutex &m);

signals:
    void imageSet();
    void imageCleared();
    void imageNull();
    void fillWidgetChanged(bool fillScrollArea);
    void droppedImagePath(QString imagePath);
    void droppedImageError();

public slots:
    void setImage(const QImage *image);
    void clearImage();
    void zoomIn(QPoint pointOfInterest);
    void zoomOut(QPoint pointOfInterest);
    void zoomFit();
    void zoomActual();
    void updateDisplayedImage();

protected:
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void wheelEvent(QWheelEvent *event) override;
    virtual void contextMenuEvent(QContextMenuEvent *event) override;
    virtual void dragEnterEvent(QDragEnterEvent *event) override;
    virtual void dragLeaveEvent(QDragLeaveEvent *event) override;
    virtual void dropEvent(QDropEvent *event) override;
    QMutex *mutex_m = nullptr;

private:
    void zoomAgain();
    void adjustScrollBar(QPoint mousePosition, float scalar = 1.0f);
    QAction *zoomInAction_m;
    QAction *zoomOutAction_m;
    QAction *zoomFitAction_m;
    QAction *zoomActualAction_m;
    QScrollArea *scrollArea_m;
    const QImage *attachedImage_m = nullptr;
    MouseWheelCtrlEaterEventFilter *wheelFilter;
    float scalar_m;
    bool fillScrollArea_m = true;
};

#endif // IMAGEWIDGET_H
