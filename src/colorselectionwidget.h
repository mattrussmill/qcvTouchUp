/***********************************************************************
* FILENAME :    colorselectionwidget.h
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
*       This is a compact, widget version of QColorDialog. It is a graphical color
*       selection widget which emits a signal for a desired color when any internal
*       widget is updated which changes the value of the color.
*
* NOTES :
*       This object was created because QColorDialog was difficult to embed into
*       another widget with no real way of removing some of its widgets.
*
* AUTHOR :  Matthew R. Miller       START DATE :    January 01/28/2019
*
* CHANGES : N/A - N/A
*
* VERSION       DATE            WHO                     DETAIL
* 0.1           02/17/2019      Matthew R. Miller       Initial Rev
*
************************************************************************/
#ifndef COLORSELECTIONWIDGET_H
#define COLORSELECTIONWIDGET_H

#include <QWidget>
class QImage;
class QPixmap;
class QColor;
class QMouseEvent;
class QPoint;

namespace Ui {
class ColorSelectionWidget;
}

class ColorSelectionWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ColorSelectionWidget(QWidget *parent = 0);
    virtual ~ColorSelectionWidget();

public slots:
    virtual void setColor(const QColor *color);

signals:
    void colorSelected(QColor);

protected slots:
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;

private:
    void paintCursorOnPalette();
    Ui::ColorSelectionWidget *ui;
    QImage valueDisplay_m;
    QPoint palettePoint_m;
    QColor selectedColor_m;

private slots:
    void populateColorValues();
    void setColorFromPalette();
    void setColorFromRGB();
    void setColorFromHSV();
    void setColorFromHTML();
    void setCursorFromColor();
};

#endif // COLORSELECTIONWIDGET_H
