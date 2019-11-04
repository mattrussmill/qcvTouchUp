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

private:
    QPoint getPointInImage(QMouseEvent *event);
    QRect getAdjustedImageRegion();
    QRect getAdjustedWidgetRegion();
    void leftMousePressEvent(QMouseEvent *event);
    void leftMouseMoveEvent(QMouseEvent *event);
    void leftMouseReleaseEvent(QMouseEvent *event);
    void setPointFromImage(); //getPointInImage from ImageWidget
    QPoint imageDragStart_m;
    QPoint widgetDragStart_m;
    QRect imageRegion_m;
    QRect widgetRegion_m;
    uchar brushRadius_m;
    float scaleWidth_m = 0;
    float scaleHeight_m = 0;
    uint retrieveCoordinateMode_m = NoClick;
    bool startPainting;
};

#endif // IMAGELABEL_H
