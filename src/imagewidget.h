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
************************************************************************/

#ifndef IMAGEWIDGET_H
#define IMAGEWIDGET_H

#include <QWidget>
class QLabel;
class QScrollArea;
class QImage;
class QMenu;
class QPoint;
class QMutex;

class ImageWidget : public QWidget
{
    Q_OBJECT

public:
    ImageWidget(QWidget *parent = nullptr);
    QPoint lastPointSelected() const;
    Qt::ScrollBarPolicy verticalScrollBarPolicy() const;
    Qt::ScrollBarPolicy horizontalScrollBarPolicy() const;
    void setVerticalScrollBarPolicy(Qt::ScrollBarPolicy sbp = Qt::ScrollBarAsNeeded);
    void setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy sbp = Qt::ScrollBarAsNeeded);
    void setImage(const QImage &image);
    void setFillWidget(bool fill = true);
    double currentScale() const;
    bool imageAttached() const;
    bool fillWidgetStatus() const;
    const QImage* displayedImage();
    void setMutex(QMutex &m);


signals:
    void imageSet();
    void imageCleared();
    void imageNull();
    void imagePointSelected(QPoint selectedPoint);
    void fillWidgetChanged(bool fillScrollArea);
    void droppedImagePath(QString imagePath);
    void droppedImageError();

public slots:
    void setImage(const QImage *image);
    void clearImage();
    void zoomIn();
    void zoomOut();
    void zoomFit();
    void zoomActual();
    void updateDisplayedImage();
    void updateDisplayedImage(const QImage *image);


protected:
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void contextMenuEvent(QContextMenuEvent *event) override;
    virtual void dragEnterEvent(QDragEnterEvent *event) override;
    virtual void dragLeaveEvent(QDragLeaveEvent *event) override;
    virtual void dropEvent(QDropEvent *event) override;

private:
    QPoint selectedPoint;
    QAction *zoomInAction;
    QAction *zoomOutAction;
    QAction *zoomFitAction;
    QAction *zoomActualAction;
    QLabel *imageLabel;
    QScrollArea *scrollArea;
    QMutex *mutex;
    const QImage *attachedImage = nullptr;
    float scalar;
    bool fillScrollArea = true;
    bool retrieveColor = false;
};

#endif // IMAGEWIDGET_H
